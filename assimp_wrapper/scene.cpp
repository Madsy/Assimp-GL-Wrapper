#include <assert.h>
#include "scene.h"
#include "png_loader.h"
#include "glstuff.h"

Scene::Scene(const std::string& path)
{
	m_Scene = importScene(path);
	initGLModelData();
}
Scene::~Scene()
{
	aiReleaseImport(m_Scene);
}

const aiScene* Scene::getScene() const 
{
	return m_Scene;
}

const aiAnimation* Scene::getAnimation(const std::string& name) const
{
	std::map<std::string, const aiAnimation*>::const_iterator it = m_LUTAnimation.find(name);
	if(it == m_LUTAnimation.end())
		return 0;
	return it->second;
}	

const MeshGLData* Scene::getMeshGLData(int idx) const
{
	if(idx >= m_MeshData.size())
		return 0;
	return m_MeshData[idx];
}
	
const aiScene* Scene::importScene(const std::string& path)
{
	const aiScene* assimpScene = 0;
	assimpScene = aiImportFile( path.c_str(), 
								aiProcess_CalcTangentSpace       | 
								aiProcess_Triangulate            |
								aiProcess_JoinIdenticalVertices  |
								aiProcess_SortByPType); // aiProcess_FlipUVs
	return assimpScene;
}

void Scene::initGLModelData()
{
	assert(m_Scene != 0);
	for(int i = 0; i < m_Scene->mNumMeshes; ++i){
		MeshGLData* glData = new MeshGLData;
		const aiMesh* mesh = m_Scene->mMeshes[i];
		std::string name(mesh->mName.C_Str());

		//Assimp supports multiple primitives, but we only want
		//triangles. So we have to convert it into a simple 1D array
		unsigned int numVertexIndices = mesh->mNumFaces * 3;
		unsigned int* indexArrayTmp = new unsigned int[numVertexIndices];
		for(int j = 0; j < mesh->mNumFaces; ++j){
			const aiFace& face = mesh->mFaces[j];
			//assert(face.mNumIndices == 3);
			if(face.mNumIndices < 3) continue;
			indexArrayTmp[(j*3) + 0] = face.mIndices[0];
			indexArrayTmp[(j*3) + 1] = face.mIndices[1];
			indexArrayTmp[(j*3) + 2] = face.mIndices[2];
		}

		// By default we have no VBOs except for vertices and indices
		glData->normals    = ~0u;
		glData->tangents   = ~0u;
		glData->bitangents = ~0u;
		glData->tcoord0    = ~0u;
		glData->tcoord1    = ~0u;
		glData->tcoord2    = ~0u;
		glData->tcoord3    = ~0u;

		glData->vao = createVAO();
		glData->vertices   = createVBO(mesh->mVertices, mesh->mNumVertices);
		if(mesh->HasNormals())
			glData->normals    = createVBO(mesh->mNormals, mesh->mNumVertices);
		if(mesh->HasTangentsAndBitangents()){
			glData->tangents   = createVBO(mesh->mTangents, mesh->mNumVertices);
			glData->bitangents = createVBO(mesh->mBitangents, mesh->mNumVertices);
		}
		glData->indices    = createVBO(indexArrayTmp, numVertexIndices);
		//used by glDrawElements in the renderer
		glData->numElements = numVertexIndices;


		unsigned int numUVMaps = mesh->GetNumUVChannels();
		if(numUVMaps > Scene::MAX_UVMAPS) numUVMaps = MAX_UVMAPS;
		//assert(numUVMaps > 0);
	
		switch(numUVMaps){
		case 4:	
			glData->tcoord3 = createVBO(mesh->mTextureCoords[3], mesh->mNumVertices);
		case 3:
			glData->tcoord2 = createVBO(mesh->mTextureCoords[2], mesh->mNumVertices);
		case 2:
			glData->tcoord1 = createVBO(mesh->mTextureCoords[1], mesh->mNumVertices);
		case 1:
			glData->tcoord0 = createVBO(mesh->mTextureCoords[0], mesh->mNumVertices);
		}

		/* How to compute the indices to the matrices and the weights?
		   We know that each mesh has its own skeleton, if any. It's
		   either a sibling of the mesh node, or a child of the mesh
		   node.
		   
		   In order words, it's guaranteed that each mesh only has one
		   skeleton (no instancing of meshes with armatures). 

		   So look up the bones in
		   m_Scene->mMeshes[i]->mBones[j]. At this point, the bone
		   node isn't important. mBones[j] has a aiVertexWeight array.
		   It stores mVertexId and mWeight for each vertex. So the
		   steps become:

		   1.) Iterate over the mBones[j] and add 'j' to boneIndices[k]
		   if mVertexID is the current vertex. Add the weight to
		   weights[k] as well.
		   2.) When later computing bone matrices, associate 'j' with
		   the node belonging to mBones[j]

		 */
		if(mesh->HasBones()){
			initGLBoneData(glData, i);
		} else {
			//0xffffffff means "no vbo"
			glData->boneIndices = ~0u;
			glData->weights = ~0u;
		}

		//Add new GL mesh data to list
		m_MeshData.push_back(glData);
	}	
}

void Scene::initGLBoneData(MeshGLData* gldata, int meshID)
{
	std::vector<std::vector<float> > weightArray; //one weight per bone
	std::vector<std::vector<unsigned int> > boneArray; //one index per bone found

	const aiMesh* mesh = m_Scene->mMeshes[meshID];
	weightArray.resize(mesh->mNumVertices);
	boneArray.resize(mesh->mNumVertices);
	assert(mesh->mNumBones <= Scene::MAXBONESPERMESH);

	int numBones = mesh->mNumBones;
	if(numBones > Scene::MAXBONESPERMESH)
		numBones = Scene::MAXBONESPERMESH;
	for(int i = 0; i < numBones; ++i){
		const aiBone* bone = mesh->mBones[i];
		for(int j = 0; j < bone->mNumWeights; ++j){
			int vertexIdx = bone->mWeights[j].mVertexId;
			float weight  = bone->mWeights[j].mWeight;
			boneArray[vertexIdx].push_back(i);
			weightArray[vertexIdx].push_back(weight);
		}
		//Associate bone 'i' with its node so we can later easily
		//check if a bone needs updating. NOTE! This assumes that
		//multiple bones can not have the same name or refer to the
        //same node!
        //Update: Got a model where a node refer to a bone shared by several meshes
		//m_LUTBone now refers to an array of nmbis
		NodeMeshBoneIndex nmbi;
		nmbi.meshIndex = meshID;
		nmbi.boneIndex = i;
		const aiNode* boneNode = m_Scene->mRootNode->FindNode(bone->mName);
		assert(boneNode != 0);
		//m_LUTBone.insert(std::make_pair(boneNode, nmbi));
		//A node which refers to a bone, can refer to a bone shared by
		//several meshes. If several meshes share a bone, the bone can
		//have different indices (different bone array ordering)
		m_LUTBone[boneNode].push_back(nmbi);
	}

	assert(boneArray.size() == mesh->mNumVertices);
	assert(weightArray.size() == mesh->mNumVertices);	
	
	// Assert for greater bones-per-vertex limit than allowed
#if 0
	std::vector<float> cmpVal;
	cmpVal.resize(Scene::MAXBONESPERVERTEX);
	assert(
		std::upper_bound(
			weightArray.begin(), weightArray.end(), cmpVal,
			[](const std::vector<float>& v1, const std::vector<float>& v2) -> bool {
				return v1.size() < v2.size();
			}) == weightArray.end());
	
#endif

	std::vector<unsigned int> boneArrayFinal;
	std::vector<float> weightArrayFinal;

	boneArrayFinal.resize(mesh->mNumVertices * Scene::MAXBONESPERVERTEX);
	weightArrayFinal.resize(mesh->mNumVertices * Scene::MAXBONESPERVERTEX);

	/* Finally, create a flat array for OpenGL
	   OBS: Currently this code expects Scene::MAXBONESPERVERTEX to be 4!
	*/
	for(int i = 0; i < mesh->mNumVertices; ++i){
		int idx = i*Scene::MAXBONESPERVERTEX;
		const std::vector<unsigned int>& ba = boneArray[i];
		const std::vector<float>& wa = weightArray[i];
		assert(ba.size() <= Scene::MAXBONESPERVERTEX);
		assert(wa.size() <= Scene::MAXBONESPERVERTEX);
		
		int len = ba.size();
		/* TODO: Fix this switch so it works with different scene
		   limits */ 
		switch(len){
		case 1:
			boneArrayFinal[idx + 0] = ba[0];
			boneArrayFinal[idx + 1] = 0;
			boneArrayFinal[idx + 2] = 0;
			boneArrayFinal[idx + 3] = 0;
			weightArrayFinal[idx + 0] = wa[0];
			weightArrayFinal[idx + 1] = 0;
			weightArrayFinal[idx + 2] = 0;
			weightArrayFinal[idx + 3] = 0;
			break;
		case 2:
			boneArrayFinal[idx + 0] = ba[0];
			boneArrayFinal[idx + 1] = ba[1];
			boneArrayFinal[idx + 2] = 0;
			boneArrayFinal[idx + 3] = 0;
			weightArrayFinal[idx + 0] = wa[0];
			weightArrayFinal[idx + 1] = wa[1];
			weightArrayFinal[idx + 2] = 0;
			weightArrayFinal[idx + 3] = 0;
			break;
		case 3:
			boneArrayFinal[idx + 0] = ba[0];
			boneArrayFinal[idx + 1] = ba[1];
			boneArrayFinal[idx + 2] = ba[2];
			boneArrayFinal[idx + 3] = 0;
			weightArrayFinal[idx + 0] = wa[0];
			weightArrayFinal[idx + 1] = wa[1];
			weightArrayFinal[idx + 2] = wa[2];
			weightArrayFinal[idx + 3] = 0;
			break;
		case 4:
			boneArrayFinal[idx + 0] = ba[0];
			boneArrayFinal[idx + 1] = ba[1];
			boneArrayFinal[idx + 2] = ba[2];
			boneArrayFinal[idx + 3] = ba[3];
			weightArrayFinal[idx + 0] = wa[0];
			weightArrayFinal[idx + 1] = wa[1];
			weightArrayFinal[idx + 2] = wa[2];
			weightArrayFinal[idx + 3] = wa[3];
			break;
		default:
			assert(false);
		}
	}
	gldata->boneIndices = createVBO(&boneArrayFinal[0], mesh->mNumVertices * 4);
	gldata->weights = createVBO(&weightArrayFinal[0], mesh->mNumVertices * 4);
}

AnimGLData* Scene::createAnimation(const std::string& name, const aiMatrix4x4& camera)
{
	AnimGLData* animation = new AnimGLData;
	animation->m_Scene = this;
	animation->m_Animation = 0;
	animation->m_Renderer = 0;
	animation->m_Bones.resize(m_Scene->mNumMeshes);
	animation->m_ModelView.resize(m_Scene->mNumMeshes);
	animation->m_Time = 0.0f;
	animation->m_Camera = camera;
	
	
	/* Linear search for animation name */
	for(int i = 0; i < m_Scene->mNumAnimations; ++i){
		std::string animName(m_Scene->mAnimations[i]->mName.C_Str());
		if(animName == name){
			animation->m_Animation = m_Scene->mAnimations[i];
			break;
		}
	}
	assert(animation->m_Animation != 0);
	

	/* Resize bone array for each model properly */
	for(int i = 0; i < m_Scene->mNumMeshes; ++i){
		//Set the number of bones to the max limit
		//GLSL requires an array of constant size
		int numBones = Scene::MAXBONESPERMESH; //m_Scene->mMeshes[i]->mNumBones;
		animation->m_Bones[i].resize(numBones);
	}	
	
	//Initial update of all the matrices in the node structure, so the
	//first rendered frame works. Implicitly reads animation->m_Time,
	//which starts at 0.0f
	aiMatrix4x4 rootMatrix;
	animation->recursiveUpdate(m_Scene->mRootNode, rootMatrix);
	return animation;
}


AnimGLData* Scene::createAnimation(unsigned int anim, const aiMatrix4x4& camera)
{
	AnimGLData* animation = new AnimGLData;
	animation->m_Scene = this;
	animation->m_Animation = 0;
	animation->m_Renderer = 0;
	animation->m_Bones.resize(m_Scene->mNumMeshes);
	animation->m_ModelView.resize(m_Scene->mNumMeshes);
	animation->m_Time = 0.0f;
	animation->m_Camera = camera;
	
	

	animation->m_Animation = m_Scene->mAnimations[anim];
	assert(animation->m_Animation != 0);
	

	/* Resize bone array for each model properly */
	for(int i = 0; i < m_Scene->mNumMeshes; ++i){
		//Set the number of bones to the max limit
		//GLSL requires an array of constant size
		int numBones = Scene::MAXBONESPERMESH; //m_Scene->mMeshes[i]->mNumBones;
		animation->m_Bones[i].resize(numBones);
	}	
	
	//Initial update of all the matrices in the node structure, so the
	//first rendered frame works. Implicitly reads animation->m_Time,
	//which starts at 0.0f
	aiMatrix4x4 rootMatrix;
	animation->recursiveUpdate(m_Scene->mRootNode, rootMatrix);
	return animation;
}



/****************************************************************************************
 ********************************* AnimRenderer *****************************************
 ****************************************************************************************/
AnimRenderer::AnimRenderer() : m_Parent(0), m_Scene(0), m_CurrentMesh(-1) {

}

void AnimRenderer::drawObjectBegin(unsigned int shader, const std::string& objname)
{
	int i;
	const aiScene* sceneData = m_Scene->m_Scene;
	// TODO: Do this faster than O(n). Maybe add a hashmap 
	for(i = 0; i < sceneData->mNumMeshes; ++i){
		std::string meshName(sceneData->mMeshes[i]->mName.C_Str());
		if(meshName == objname)
			break;
	}
	assert(i < sceneData->mNumMeshes);
	m_CurrentMesh = i;
	const MeshGLData* meshData = m_Scene->getMeshGLData(m_CurrentMesh);

	glUseProgram(shader);
	bindVAO(meshData->vao);
	bindVBOFloat(shader, "sc_vertex",     meshData->vertices,   3);
	bindVBOFloat(shader, "sc_normal",     meshData->normals,    3);
	bindVBOFloat(shader, "sc_tangent",    meshData->tangents,   3);
	bindVBOFloat(shader, "sc_bitangent",  meshData->bitangents, 3);
	bindVBOFloat(shader, "sc_tcoord0",    meshData->tcoord0,    3);
	bindVBOFloat(shader, "sc_tcoord1",    meshData->tcoord1,    3);
	bindVBOFloat(shader, "sc_tcoord2",    meshData->tcoord2,    3);
	bindVBOFloat(shader, "sc_tcoord3",    meshData->tcoord3,    3);
	bindVBOUint( shader, "sc_index",      meshData->boneIndices,4);
	bindVBOFloat(shader, "sc_weight",     meshData->weights,    4);

	//Bone uniform array changes every frame
	//so it's stored in struct AnimGLData, this AnimRenderer's parent
	int numBones = m_Parent->m_Bones[i].size();
	const std::vector<aiMatrix4x4>& bones = m_Parent->m_Bones[m_CurrentMesh];
	bindUniformMatrix4Array(shader, "sc_bones", numBones, &bones[0]);
	bindUniformMatrix4(shader, "sc_modelview", m_Parent->m_ModelView[m_CurrentMesh]);
	bindUniformMatrix4(shader, "sc_camera", m_Parent->m_Camera);
	//Finally, bind the face indices
	bindVBOIndices(shader, meshData->indices);
}

void AnimRenderer::drawObjectEnd()
{
	assert(m_CurrentMesh != -1);
	const MeshGLData* meshData = m_Scene->getMeshGLData(m_CurrentMesh);
	glDrawElements(GL_TRIANGLES, meshData->numElements, GL_UNSIGNED_INT, 0);
	m_CurrentMesh = -1;
}

void AnimRenderer::drawAllObjects(unsigned int shader)
{
	glUseProgram(shader);
	const aiScene* sceneData = m_Scene->m_Scene;
	for(int i = 0; i <sceneData->mNumMeshes; ++i){
		m_CurrentMesh = i;
		const MeshGLData* meshData = m_Scene->getMeshGLData(m_CurrentMesh);
		bindVAO(meshData->vao);
		bindVBOFloat(shader, "sc_vertex",     meshData->vertices,   3);
		bindVBOFloat(shader, "sc_normal",     meshData->normals,    3);
		bindVBOFloat(shader, "sc_tangent",    meshData->tangents,   3);
		bindVBOFloat(shader, "sc_bitangent",  meshData->bitangents, 3);
		bindVBOFloat(shader, "sc_tcoord0",    meshData->tcoord0,    3);
		bindVBOFloat(shader, "sc_tcoord1",    meshData->tcoord1,    3);
		bindVBOFloat(shader, "sc_tcoord2",    meshData->tcoord2,    3);
		bindVBOFloat(shader, "sc_tcoord3",    meshData->tcoord3,    3);
		bindVBOUint( shader, "sc_index",      meshData->boneIndices,4);
		bindVBOFloat(shader, "sc_weight",     meshData->weights,    4);

		//Bone uniform array changes every frame
		//so it's stored in struct AnimGLData, this AnimRenderer's parent
		int numBones = m_Parent->m_Bones[i].size();
		const std::vector<aiMatrix4x4>& bones = m_Parent->m_Bones[i];
		bindUniformMatrix4Array(shader, "sc_bones", numBones, &bones[0]);
		bindUniformMatrix4(shader, "sc_modelview", m_Parent->m_ModelView[m_CurrentMesh]);
		bindUniformMatrix4(shader, "sc_camera", m_Parent->m_Camera);


		const aiMatrix4x4& c = m_Parent->m_Camera;
		const float* cp = c[0];
		
		//Finally, bind the face indices
		bindVBOIndices(shader, meshData->indices);
		glDrawElements(GL_TRIANGLES, meshData->numElements, GL_UNSIGNED_INT, 0);
	}
	m_CurrentMesh = -1;
}

void AnimRenderer::draw()
{
	//override this and use drawObjectBegin()/drawObjectEnd and drawAllObjects() as needed
}

void AnimRenderer::setParent(AnimGLData* parent)
{
	m_Parent = parent;
}

void AnimRenderer::setScene(const Scene* scene)
{
	m_Scene = scene;
}



/****************************************************************************************
 ********************************* AnimGLData** *****************************************
 ****************************************************************************************/
void AnimGLData::addRenderer(AnimRenderer* renderer)
{
	renderer->setParent(this);
	renderer->setScene(m_Scene);
	m_Renderer = renderer;
}

void AnimGLData::removeRenderer()
{
	m_Renderer = 0; //cleanup outside of this renderer
}

void AnimGLData::stepAnimation(float t) //step one frame forwards
{
	const aiScene* sceneData = m_Scene->m_Scene;
	float step;
	if(m_Animation->mTicksPerSecond != 0.0f)
		step = m_Animation->mTicksPerSecond;
	else
		step = 32.0f;

	m_Time = t * step; //Used as time position by recursiveUpdate

	//Run recursive node updates here, with current camera
	recursiveUpdate(sceneData->mRootNode, m_Camera);
}

void AnimGLData::render(float t)
{
	stepAnimation(t);
	if(m_Renderer)
		m_Renderer->draw();
}

void AnimGLData::setCamera(const aiMatrix4x4& camera)
{
	m_Camera = camera;
}


//For an animated node (an aiNodeAnim channel), get the interpolated position
void AnimGLData::interpolateTranslation(const aiNodeAnim* nodeAnim, aiVector3D& translation)
{
	bool positionFound = false;
	for(int i = 0; i < nodeAnim->mNumPositionKeys - 1; ++i){
		const aiVectorKey& key1 = nodeAnim->mPositionKeys[i];
		const aiVectorKey& key2 = nodeAnim->mPositionKeys[i+1];		
		if(key1.mTime <= m_Time && m_Time <= key2.mTime){
			float tDelta = key2.mTime - key1.mTime;
			float t = (m_Time - key1.mTime) / tDelta;
			//float t = m_Time - key1.mTime;
			translation[0] = key1.mValue[0] + (key2.mValue[0] - key1.mValue[0])*t;
			translation[1] = key1.mValue[1] + (key2.mValue[1] - key1.mValue[1])*t;
			translation[2] = key1.mValue[2] + (key2.mValue[2] - key1.mValue[2])*t;
			positionFound = true;
			break;
		}			
	}
	// If the time isn't inside the animation channel timeframe, snap to the
	//   closest frame, which is either the first or the last
	//   frame.
	// Another solution would be to compute the length, and do a
	//   modulo on the time so it repeats
	if(!positionFound){
		float firstFrameTime = nodeAnim->mPositionKeys[0].mTime;
		float lastFrameTime = nodeAnim->mPositionKeys[nodeAnim->mNumPositionKeys - 1].mTime;
		const aiVector3D& firstFrameValue = nodeAnim->mPositionKeys[0].mValue;
		const aiVector3D& lastFrameValue = nodeAnim->mPositionKeys[nodeAnim->mNumPositionKeys - 1].mValue;
		if(m_Time < firstFrameTime){
			translation = firstFrameValue;
		} else if(m_Time > lastFrameTime){
			translation = lastFrameValue;
		} else {
			//Shouldn't get here. Either we find a time between
			//two frames, or we get to pick the first or last frame
			assert(false);
		}			
	}
}

//For an animated node (an aiNodeAnim channel), get the interpolated scale
void AnimGLData::interpolateScale(const aiNodeAnim* nodeAnim, aiVector3D& scale)
{
	bool scaleFound = false;
	for(int i = 0; i < nodeAnim->mNumScalingKeys - 1; ++i){
		const aiVectorKey& key1 = nodeAnim->mScalingKeys[i];
		const aiVectorKey& key2 = nodeAnim->mScalingKeys[i+1];
		if(key1.mTime <= m_Time && m_Time <= key2.mTime){
			float tDelta = key2.mTime - key1.mTime;
			float t = (m_Time - key1.mTime) / tDelta;
			//float t = m_Time - key1.mTime;
			scale[0] = key1.mValue[0] + (key2.mValue[0] - key1.mValue[0])*t;
			scale[1] = key1.mValue[1] + (key2.mValue[1] - key1.mValue[1])*t;
			scale[2] = key1.mValue[2] + (key2.mValue[2] - key1.mValue[2])*t;
			scaleFound = true;
			break;
		}			
	}
	// If the time isn't inside the animation channel timeframe, snap to the
	//   closest frame, which is either the first or the last
	//   frame.
	// Another solution would be to compute the length, and do a
	//   modulo on the time so it repeats
	if(!scaleFound){
		float firstFrameTime = nodeAnim->mScalingKeys[0].mTime;
		float lastFrameTime = nodeAnim->mScalingKeys[nodeAnim->mNumScalingKeys - 1].mTime;
		const aiVector3D& firstFrameValue = nodeAnim->mScalingKeys[0].mValue;
		const aiVector3D& lastFrameValue = nodeAnim->mScalingKeys[nodeAnim->mNumScalingKeys - 1].mValue;
		if(m_Time < firstFrameTime){
			scale = firstFrameValue;
		} else if(lastFrameTime){
			scale = lastFrameValue;
		} else {
			//Shouldn't get here. Either we find a time between
			//two frames, or we get to pick the first or last frame
			assert(false);
		}			
	}
}

//For an animated node (an aiNodeAnim channel), get the interpolated rotation
void AnimGLData::interpolateRotation(const aiNodeAnim* nodeAnim, aiQuaternion& rotation)
{
	bool rotationFound = false;
	// TODO: Make this faster than O(n)
	for(int i = 0; i < nodeAnim->mNumRotationKeys - 1; ++i){
		const aiQuatKey& key1 = nodeAnim->mRotationKeys[i];
		const aiQuatKey& key2 = nodeAnim->mRotationKeys[i+1];
		//printf("rot key timestamps: %lf %lf\n", key1.mTime, key2.mTime);
		if(key1.mTime <= m_Time && m_Time <= key2.mTime){
			float tDelta = key2.mTime - key1.mTime;
			float t = (m_Time - key1.mTime) / tDelta;
			//float t = m_Time - key1.mTime;
			aiQuaternion::Interpolate(rotation, key1.mValue, key2.mValue, t);
			//rotation = key1.mValue;
			rotationFound = true;
			break;
		}			
	}
	// If the time isn't inside the channel timeframe, snap to the
	//   closest frame, which is either the first or the last
	//   frame.
	// Another solution would be to compute the length, and do a
	//   modulo on the time so it repeats
	if(!rotationFound){
		float firstFrameTime = nodeAnim->mRotationKeys[0].mTime;
		float lastFrameTime = nodeAnim->mRotationKeys[nodeAnim->mNumRotationKeys - 1].mTime;
		const aiQuaternion& firstFrameValue = nodeAnim->mRotationKeys[0].mValue;
		const aiQuaternion& lastFrameValue = nodeAnim->mRotationKeys[nodeAnim->mNumRotationKeys - 1].mValue;
/*
		aiQuaternion firstFrameValue = nodeAnim->mRotationKeys[0].mValue;
		aiQuaternion lastFrameValue = nodeAnim->mRotationKeys[nodeAnim->mNumRotationKeys - 1].mValue;
		firstFrameValue.Normalize();
		lastFrameValue.Normalize();
*/
		if(m_Time < firstFrameTime){
			rotation = firstFrameValue;
		} else if(m_Time > lastFrameTime){
			rotation = lastFrameValue;
		} else {
			//Shouldn't get here. Either we find a time between
			//two frames, or we get to pick the first or last frame
			assert(false);
		}			
	}
}

/* Recursively update the coordinate systems of nodes, including bones */
void AnimGLData::recursiveUpdate(aiNode* node, const aiMatrix4x4& parentMatrix)
{
	std::string nodeName(node->mName.C_Str());
	aiMatrix4x4 localMatrix = node->mTransformation;
	
	//find this current node in the animation
	//TODO: Make this faster than O(n)
	//Note: setting the m_Animation pointer to 0 effectively disables animation
	const aiNodeAnim* nodeAnim = 0;	
	for(int i = 0; i < m_Animation->mNumChannels && m_Animation; ++i){
		aiNodeAnim* anim = m_Animation->mChannels[i];
		std::string animNodeName(anim->mNodeName.C_Str());
		if(animNodeName == nodeName){
			nodeAnim = anim;
			break;
		}		
	}

	// Animate this node if we found an animation channel for it earlier
	// Replaces localMatrix
	if(nodeAnim){		
		aiVector3D translation;
		aiVector3D scale;
		aiQuaternion rotation;
		aiMatrix4x4 scaleMat, rotMat, transMat;
		//printf("Animated node: %s at time %f\n", node->mName.C_Str(), m_Time);
		interpolateTranslation(nodeAnim, translation);
		interpolateScale(nodeAnim, scale);
		interpolateRotation(nodeAnim, rotation);
		rotMat = aiMatrix4x4(rotation.GetMatrix());
		aiMatrix4x4::Scaling(scale, scaleMat);
		aiMatrix4x4::Translation(translation, transMat);
		localMatrix = transMat * rotMat; // * scaleMat;
	}

	aiMatrix4x4 globalMatrix = parentMatrix * localMatrix;

	/* Global world transform for meshes in pose mode (no animation running) */
	for(int i = 0; i < node->mNumMeshes; ++i)
	{
		m_ModelView[node->mMeshes[i]]  = globalMatrix;
	}
	
	/* Look up node in NMBI lookup table. If it is a bone, update the
	i'th bone in the j'th mesh. The "bone" we update is the 2D matrix
	array used by OpenGL as uniforms. Each array in the 2D array
	belongs to a mesh. */
	bool isBone = false;
	const aiScene* sceneData = m_Scene->m_Scene;
	std::map<const aiNode*, std::vector<NodeMeshBoneIndex> >::const_iterator it = m_Scene->m_LUTBone.find(node);
	isBone = (it != m_Scene->m_LUTBone.end());
	if(isBone){
		const std::vector<NodeMeshBoneIndex>& nmbi = it->second;
		
		for(unsigned int i = 0; i < nmbi.size(); ++i){
			const NodeMeshBoneIndex& idx = nmbi[i];
			const aiMatrix4x4& offsetMatrix = sceneData->mMeshes[idx.meshIndex]->mBones[idx.boneIndex]->mOffsetMatrix;
			aiMatrix4x4 boneMatrix = globalMatrix * offsetMatrix;
			//Update the 'nmbi.meshIndex'th mesh, bone number 'nmbi.boneIndex' 
			//OpenGL uses one uniform array for each mesh as bone matrices
			//Now we support that a bone can be shared by multiple meshes
			
			m_Bones[idx.meshIndex][idx.boneIndex] = boneMatrix;
		}
		
	}
	for(int i = 0; i < node->mNumChildren; ++i)
		recursiveUpdate(node->mChildren[i], globalMatrix);
}

