#ifndef SCENE_H
#define SCENE_H

#include <assimp/cimport.h>
#include <assimp/scene.h> 
#include <assimp/postprocess.h>
#include <assimp/types.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <fstream>

/* 
   aiScene have aiMeshes and aiAnimations
   aiMesh have aiBones.
   aiBones in aiMeshes point to aiNodes, which define the coordinate system.
   An aiAnimation have an array of channels of type aiNodeAnim which
   each animate an aiNode, including bones.

   
*/

/* All this OpenGL data is constant during animation */
struct MeshGLData
{
	unsigned int vao;
	/* vertex buffer objects */
	unsigned int vertices;
	unsigned int normals;
	unsigned int tangents;
	unsigned int bitangents;
	unsigned int tcoord0;
	unsigned int tcoord1;
	unsigned int tcoord2;
	unsigned int tcoord3;
	unsigned int indices; //vertex indices to glDrawElements 
	unsigned int boneIndices; //indices to bones affecting a vertex
	unsigned int weights; //bone weights
	unsigned int numElements; //number of faces * 3
	/* uniforms */
	//std::vector<aiMatrix4x4> bones; //final bones after transformation
};

struct AnimGLData;
struct Scene;

/* Abstract away rendering in AnimGLData so we can use custom render
 * functions and multiple passes */
struct AnimRenderer
{
	AnimRenderer();
	friend class AnimGLData;
protected:
	void drawObjectBegin(unsigned int shader, const std::string& objname);
	void drawObjectEnd();
	void drawAllObjects(unsigned int shader);
private:
	void setParent(AnimGLData* parent);
	void setScene(const Scene* scene);
	virtual void draw();
	int m_CurrentMesh;
	AnimGLData* m_Parent;
	const Scene* m_Scene;
};	
	
/* This OpenGL data is dynamic during animation. This struct lets us
 * create multiple instances of an animation with different time offsets. */
struct AnimGLData
{
	friend class Scene;
	const Scene* m_Scene;
	//pointer to the animation data (constant)
	const aiAnimation* m_Animation;
	AnimRenderer* m_Renderer;
	//2D array of uniform matrices for bones for every mesh (changes every frame)
	std::vector<std::vector<aiMatrix4x4> > m_Bones;
	//One worldspace matrix for every mesh
	std::vector<aiMatrix4x4> m_ModelView;
	//time of animation
	float m_Time;
	aiMatrix4x4 m_Camera;
	
	void addRenderer(AnimRenderer* renderer);
	void removeRenderer();
	void stepAnimation(float t); //step one frame forwards
	void render(float t);
	void setCamera(const aiMatrix4x4& camera);
private:
	void recursiveUpdate(aiNode* node, const aiMatrix4x4& parentMatrix);
	void interpolateTranslation(const aiNodeAnim* nodeAnim, aiVector3D& translation);
	void interpolateScale(const aiNodeAnim* nodeAnim, aiVector3D& scale);
	void interpolateRotation(const aiNodeAnim* nodeAnim, aiQuaternion& rotation);
	
};

/* Could have used an std::pair, but a new type is more readable.
 * This struct is stored per-node if it is a bone, so we can look up
 * the model and bone index */
struct NodeMeshBoneIndex
{
	int meshIndex;
	int boneIndex;
};

struct Scene
{
	static const int MAX_UVMAPS = 4;
	static const int MAXBONESPERVERTEX = 4;
	static const int MAXBONESPERMESH = 32;
	
	const aiScene* m_Scene;
	//look up animations by name
	std::map<std::string, const aiAnimation*> m_LUTAnimation;
	//look up bone ID and Mesh ID by node. I.e aiNode* 'node' is the 'i'th bone
	//in the 'j'th mesh.
	std::map<const aiNode*, std::vector<NodeMeshBoneIndex> > m_LUTBone;
	//Constant/static data used by OpenGL for each mesh
	std::vector<MeshGLData*> m_MeshData;
	//Dynamic animation data per animation instance that changes every
	//animation frame
	std::vector<AnimGLData*> m_AnimData;

	//functions
	Scene(const std::string& path);
	~Scene();
	const aiScene* getScene() const;
	const aiAnimation* getAnimation(const std::string& name) const;
	const MeshGLData* getMeshGLData(int idx) const;
	AnimGLData* createAnimation(const std::string& name, const aiMatrix4x4& camera);
	AnimGLData* createAnimation(unsigned int anim, const aiMatrix4x4& camera);
private:
	const aiScene* importScene(const std::string& path);
	void initGLModelData();
	void initGLBoneData(MeshGLData* gldata, int meshID);
};

#endif
