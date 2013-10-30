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
#include <iostream>
#include <sstream>

struct NodeMeshBoneIndex
{
	int meshIndex;
	int boneIndex;
};

//std::map<const aiNode*, int> meshLUT;
std::map<const aiNode*, NodeMeshBoneIndex> boneLUT;
std::map<const aiNode*, int> lightLUT;
std::map<const aiNode*, int> cameraLUT;

/* A node can be referenced by the channels of several animations */
std::map<const aiNode*, std::vector<int> > animationLUT;

std::vector<std::string> node_hierarcy; //"foo -> bar" listings
std::vector<std::string> node_hierarcy_info; //labels for graphs;
												 //the actual
												 //information

const aiScene* importScene(const std::string& path);
void printDotFileGraph(const aiScene* scene, const std::string& outPath);
void printNodes(const aiNode* node, const std::string& dotName);
void printDotNode(const std::string& nodeName, const std::string& label);
void printDotConnection(const std::string& node1, const std::string& node2);

int main(int argc, char* argv[])
{
	using std::cout;
	using std::endl;
	using std::ofstream;
	
	if(argc != 3){
		cout << "Usage: " << argv[0] << " [collada file] [out file]" << endl;
		return 0;
	}
	
	const aiScene* scene = importScene(argv[1]);

	if(!scene){
		cout << "Couldn't open collada file, " << '\"' << argv[1] << '\"' << endl;
		return 0;
	}
	printDotFileGraph(scene, argv[2]);
	return 0;
}

const aiScene* importScene(const std::string& path)
{
	const aiScene* assimpScene = 0;
	assimpScene = aiImportFile( path.c_str(), 
								aiProcess_CalcTangentSpace       | 
								aiProcess_Triangulate            |
								aiProcess_JoinIdenticalVertices  |
								aiProcess_SortByPType); // aiProcess_FlipUVs
	return assimpScene;
}

void printDotFileGraph(const aiScene* scene, const std::string& outPath)
{
	using std::endl;
	using std::to_string;
	
	std::ofstream strm(outPath.c_str());
	if(!strm.is_open()) return;
	
	// First, update the lookup tables

	for(int i = 0; i < scene->mNumLights; ++i){
		const aiNode* node = scene->mRootNode->FindNode(scene->mLights[i]->mName);
		if(node)
			lightLUT.insert(std::make_pair(node, i));
	}
	for(int i = 0; i < scene->mNumCameras; ++i){
		const aiNode* node = scene->mRootNode->FindNode(scene->mCameras[i]->mName);
		if(node)
			cameraLUT.insert(std::make_pair(node, i));
	}

	for(int i = 0; i < scene->mNumMeshes; ++i){
		const aiNode* node = scene->mRootNode->FindNode(scene->mMeshes[i]->mName);
/*
		if(node)
			meshLUT.insert(std::make_pair(node, i));
*/
		const aiMesh* mesh = scene->mMeshes[i];
		for(int j = 0; j < mesh->mNumBones; ++j){
			const aiNode* node2 = scene->mRootNode->FindNode(mesh->mBones[j]->mName);
			NodeMeshBoneIndex nmbi;
			nmbi.meshIndex = i;
			nmbi.boneIndex = j;
			if(node2)
				boneLUT.insert(std::make_pair(node2, nmbi));
		}
	}
	// animationLUT: Lookup node for reference in animation
	for(int i = 0; i < scene->mNumAnimations; ++i){
		const aiAnimation* anim = scene->mAnimations[i];
		for(int j = 0; j < anim->mNumChannels; ++j){
			const aiNode* node = scene->mRootNode->FindNode(anim->mChannels[j]->mNodeName);
			/*
			if(node)
				animationLUT.insert(std::make_pair(node, i));
			*/
			if(node){				
				std::vector<int>& arr = animationLUT[node];
				arr.push_back(i);
			}			
		}
	}

	std::ostringstream info;
	info << "Scene:" << "\\n";
	info << "Number of meshes:     " << scene->mNumMeshes << "\\n";
	info << "Number of lights:     " << scene->mNumLights << "\\n";
	info << "Number of cameras:    " << scene->mNumCameras << "\\n";
	info << "Number of animations: " << scene->mNumAnimations << "\\n";
	info << "Is scene complete? " << ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) ? "[No]" : "[Yes]" ) << "\\n";
	
	printDotNode("Node_scene_0", info.str());
	
	info.str("");
	info.clear();
	
	/* Print mesh info as nodes with identifier "Node_mesh_X",
	 where X = 0..1..2..*/
	for(int i = 0; i < scene->mNumMeshes; ++i){
		const aiMesh* mesh = scene->mMeshes[i];
		std::string nodename;

		info << "Mesh name:                     " << mesh->mName.C_Str() << "\\n";
		info << "Number of vertices:            " << mesh->mNumVertices << "\\n";
		info << "Number of vertex indices:      " << mesh->mNumFaces * 3 << "\\n";
		info << "Number of UV maps:             " << mesh->GetNumUVChannels() << "\\n";

		int numTangents = mesh->HasTangentsAndBitangents() ? mesh->mNumVertices : 0;
		info << "Number of tangents/bitangents: " << numTangents << "\\n";
		info << "Number of bones:               " << mesh->mNumBones << "\\n";
		nodename = "Node_mesh_" + to_string(i);
		printDotNode(nodename, info.str());
		info.str("");
		info.clear();
		
		for(int j = 0; j < mesh->mNumBones; ++j){
			const aiBone* bone = mesh->mBones[j];
			info << "Bone name:         " << bone->mName.C_Str() << "\\n";
			info << "Number of weights: " << bone->mNumWeights << "\\n";
			//info << "OffsetMatrix: " <<
			std::string nodename2 = "Node_mesh_" + to_string(i) + "_bone_" + to_string(j);
			printDotNode(nodename2, info.str());
			printDotConnection(nodename, nodename2);
			info.str("");
			info.clear();			
		}		
	}

	info.str("");
	info.clear();

	/* Same for animations */
	for(int i = 0; i < scene->mNumAnimations; ++i){
		std::string nodename;
		const aiAnimation* anim = scene->mAnimations[i];
		info << "Animation name:     " << anim->mName.C_Str() << "\\n";
		info << "Duration (ticks):   " << anim->mDuration << "\\n";
		info << "Ticks per second:   " << anim->mTicksPerSecond << "\\n";
		info << "Number of channels: " << anim->mNumChannels << "\\n";
		nodename = "Node_animation_" + to_string(i);
		printDotNode(nodename, info.str());
		info.str("");
		info.clear();
	}
	printNodes(scene->mRootNode, "Node_0");

	/* Write resulting string arrays to file, starting with the
	   hierarcy */
	
	strm << "digraph assimp {\n";
	
	for(unsigned int i = 0; i < node_hierarcy.size(); ++i)
		strm << node_hierarcy[i];
	strm << "\n\n\n";
	for(unsigned int i = 0; i < node_hierarcy_info.size(); ++i)
		strm << node_hierarcy_info[i];
	
	strm << "}\n";

	strm.flush();
	strm.close();
}

void printNodes(const aiNode* node, const std::string& dotName)
{
	using std::endl;
	using std::to_string;
	
//	std::map<const aiNode*, int>::const_iterator it_mesh = meshLUT.find(node);
	std::map<const aiNode*, NodeMeshBoneIndex>::const_iterator it_bone = boneLUT.find(node);
	std::map<const aiNode*, int>::const_iterator it_light = lightLUT.find(node);
	std::map<const aiNode*, int>::const_iterator it_camera = cameraLUT.find(node);

//	bool bRefMesh   = (it_mesh   != meshLUT.end());
	bool bRefMesh = (node->mNumMeshes > 0);
	bool bRefBone   = (it_bone   != boneLUT.end());
	bool bRefLight  = (it_light  != lightLUT.end());
	bool bRefCamera = (it_camera != cameraLUT.end());

	std::ostringstream info;

	info << "Name: " << node->mName.C_Str() << "\\n";

	if(bRefMesh){
		info << "References mesh(es) ";
		if(node->mNumMeshes > 1){
			int i;
			for(i = 0; i < node->mNumMeshes - 1; ++i){
				info << "\'" << node->mMeshes[i] << "\'" << ", ";
				printDotConnection("Node_mesh_" + to_string(node->mMeshes[i]), dotName);
			}
			info << "\'" << node->mMeshes[i] << "\'" << "\\n";
			printDotConnection(dotName, "Node_mesh_" + to_string(node->mMeshes[i]));
		} else {
			info << "\'" << node->mMeshes[0] << "\'" << "\\n";
			printDotConnection(dotName, "Node_mesh_" + to_string(node->mMeshes[0]));	
		}		
	}
	if(bRefBone){
		info << "References bone from mesh " << "\'" << it_bone->second.meshIndex <<
			"\'" << ", bone ID " << "\'" << it_bone->second.boneIndex << "\'" << "\\n";
	}
	if(bRefLight){
		info << "References light " << "\'" << it_light->second << "\'" << "\\n";
	}
	if(bRefCamera){
		info << "References camera " << "\'" << it_camera->second << "\'" << "\\n";
	}
	printDotNode(dotName, info.str());
	for(int i = 0; i < node->mNumChildren; ++i){
		std::string childDotName = dotName + "_" + to_string(i);
		printDotConnection(dotName, childDotName);
		printNodes(node->mChildren[i], childDotName);
	}	
}


void printDotNode(const std::string& nodeName, const std::string& label)
{
	std::string dotNode = "    " + nodeName + " " + "[" + "label=" + "\"" + label + "\"" + "];\n";
	node_hierarcy_info.push_back(dotNode);
}

void printDotConnection(const std::string& node1, const std::string& node2)
{
	std::string conn = node1 + " -> " + node2 + ";\n";
	node_hierarcy.push_back(conn);
}
