#ifndef MODEL_ANI_H
#define MODEL_ANI_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

#include "mesh_ani.h"
#include "texture.h"

glm::mat4 ToMat4_point(const aiMatrix4x4* from)
{
    glm::mat4 to;
    to[0][0] = (GLfloat)from->a1; to[0][1] = (GLfloat)from->b1;  to[0][2] = (GLfloat)from->c1; to[0][3] = (GLfloat)from->d1;
    to[1][0] = (GLfloat)from->a2; to[1][1] = (GLfloat)from->b2;  to[1][2] = (GLfloat)from->c2; to[1][3] = (GLfloat)from->d2;
    to[2][0] = (GLfloat)from->a3; to[2][1] = (GLfloat)from->b3;  to[2][2] = (GLfloat)from->c3; to[2][3] = (GLfloat)from->d3;
    to[3][0] = (GLfloat)from->a4; to[3][1] = (GLfloat)from->b4;  to[3][2] = (GLfloat)from->c4; to[3][3] = (GLfloat)from->d4;

    return to;
}

glm::mat4 ToMat4_value(const aiMatrix4x4& from)
{
    glm::mat4 to;
    //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

struct BoneInfo
{
    //aiMatrix4x4 boneOffset;
    //aiMatrix4x4 finalTransform;
    glm::mat4 finalTransform;
    glm::mat4 boneOffset;
};

class Model_ani
{
public:
    // model data 
    vector<Texture_ani> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh_ani>    meshes;
    string directory;
    bool gammaCorrection;
    const aiScene* scene;
    map<string, int> boneMap;
    vector<BoneInfo> boneInfos;


    // constructor, expects a filepath to a 3D model.
    Model_ani(string const& path, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw(Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

private:
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const& path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene2 = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        this->scene = scene2;
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh_ani processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // data to fill
        vector<Vertex_ani> vertices;
        vector<unsigned int> indices;
        vector<Texture_ani> textures;
        vector< vector <unsigned int>> ids_buffer;
        vector< vector <float>> weights_buffer;
        // walk through each of the mesh's vertices
        
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            vector <unsigned int> fogRow;
            for (int j = 0; j < 4; j++)
            {
                fogRow.push_back(0);
            }
            ids_buffer.push_back(fogRow);
        }
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            vector <float> fogRow;
            for (int j = 0; j < 4; j++)
            {
                fogRow.push_back(0.0f);
            }
            weights_buffer.push_back(fogRow);
        }
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex_ani vertex;
            
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
            // texture coordinates
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;

            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        int numBones = 0;
        int boneIndex;
        vector<BoneInfo> boneInfos_buffer;
        for (int i = 0; i < mesh->mNumBones; i++)
        {
            string BoneName = mesh->mBones[i]->mName.data;

            if (boneMap.find(BoneName) == boneMap.end()) {
                boneIndex = numBones;
                numBones++;
                BoneInfo bi;
                boneInfos_buffer.push_back(bi);
            }
            else {
                boneIndex = boneMap[BoneName];
            }

            boneMap[BoneName] = boneIndex;
            boneInfos_buffer[boneIndex].boneOffset = ToMat4_value(mesh->mBones[i]->mOffsetMatrix);
            boneInfos=boneInfos_buffer;
            
            for (int j = 0; j < mesh->mBones[i]->mNumWeights; j++)
            {
                unsigned int vertexId = mesh->mBones[i]->mWeights[j].mVertexId;
                float weight = mesh->mBones[i]->mWeights[j].mWeight;
                for (int k = 0; k < 4; k++)
                { 
                    if (weights_buffer[vertexId][k] == 0.0f)
                    {
                        ids_buffer[vertexId][k] = boneIndex;
                        weights_buffer[vertexId][k] = weight;
                        break;
                    }
                }
            }
        }

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            glm::vec4 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // ids
            vector.x = ids_buffer[i][0];
            vector.y = ids_buffer[i][1];
            vector.z = ids_buffer[i][2];
            vector.w = ids_buffer[i][3];
            vertices[i].v_boneIds = vector;
            // 
            vector.x = weights_buffer[i][0];
            vector.y = weights_buffer[i][1];
            vector.z = weights_buffer[i][2];
            vector.w = weights_buffer[i][3];
            vertices[i].v_boneWeights = vector;
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        vector<Texture_ani> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<Texture_ani> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<Texture_ani> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<Texture_ani> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        // return a mesh object created from the extracted mesh data
        return Mesh_ani(vertices, indices, textures);

    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture_ani> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Texture_ani> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip)
            {   // if texture hasn't been loaded already, load it
                Texture_ani texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }

};


class AnimationComponent {
public:
    vector<BoneInfo> boneInfos;
    const aiScene* scene;
    map<string, int> boneMap;

    /*  Functions  */
    // constructor
    AnimationComponent(const aiScene* scene, vector<BoneInfo> boneInfos, map<string, int> boneMap)
    {    
        this->scene = scene;
        this->boneMap = boneMap;
        this->boneInfos = boneInfos;
    }

    vector<glm::mat4> ExtractBoneTransforms(float animationTime)
    {
        // 애니메이션 이 계속 반복되도록 fmod 연산을 취함
        animationTime = fmod(animationTime, scene->mAnimations[0]->mDuration);
        ReadNodeHierarchy(animationTime, scene->mRootNode, glm::mat4(1.0f));
        vector<glm::mat4> boneTransforms;
        for (int i = 0; i < scene->mMeshes[0]->mNumBones; i++)
        {
            boneTransforms.push_back(boneInfos[i].finalTransform);
        }
        return boneTransforms;
    }

    void ReadNodeHierarchy(float animationTime, const aiNode* node, const glm::mat4& parentTransform)
    {
        string nodeName(node->mName.data);
        const aiAnimation* animation = scene->mAnimations[0];
        glm::mat4 nodeTransform = ToMat4_value(node->mTransformation);
        const aiNodeAnim* nodeAnim = FindNodeAnim(animation, nodeName);
        // 애니메이션 정보가 있는 node라면
        if (nodeAnim)
        {
            // 주어진 key frame의 정보와 animationTime 정보를 이용해 interpolation을 하고 값을 저장
            const aiVector3D& scaling = CalcInterpolatedValueFromKey(animationTime, nodeAnim->mNumScalingKeys, nodeAnim->mScalingKeys);
            glm::mat4 scalingM = glm::scale(glm::mat4(1.0f), glm::vec3(scaling.x, scaling.y, scaling.z));

            const aiQuaternion& rotationQ = CalcInterpolatedValueFromKey(animationTime, nodeAnim->mNumRotationKeys, nodeAnim->mRotationKeys);
            glm::mat4 rotationM = glm::mat4_cast(glm::quat(rotationQ.w, rotationQ.x, rotationQ.y, rotationQ.z));

            const aiVector3D& translation = CalcInterpolatedValueFromKey(animationTime, nodeAnim->mNumPositionKeys, nodeAnim->mPositionKeys);
            glm::mat4 translationM = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));

            nodeTransform = translationM * rotationM * scalingM;
            
        }

        // globalTransform은 bone space에서 정의되었던 정점들을 model space에서 정의되도록 함
        // parentTransform은 부모 bone space에서 정의되었던 정점들을 model space에서 정의되도록 함
        // nodeTransform은 bone space에서 정의되었던 정점들을 부모 bone space에서 정의되도록 함,
        // 혹은 부모 bone space를 기준으로 한 일종의 변환
        glm::mat4 globalTransform = parentTransform * nodeTransform;
        
        // bone이 있는 노드에 대해서만 bone Transform을 저장
        // boneMap은 map<string, int> 타입으로 bone의 이름과 index 저장
        if (boneMap.find(nodeName) != boneMap.end())
        {
            unsigned int boneIndex = boneMap[nodeName];
            boneInfos[boneIndex].finalTransform =
                ToMat4_value(scene->mRootNode->mTransformation) * globalTransform * boneInfos[boneIndex].boneOffset;
        }

        // 모든 자식 노드에 대해 재귀 호출
        for (int i = 0; i < node->mNumChildren; i++)
            ReadNodeHierarchy(animationTime, node->mChildren[i], globalTransform);
    }

    aiNodeAnim* FindNodeAnim(const aiAnimation* animation, const string nodeName)
    {
     
        for (int i = 0; i < animation->mNumChannels; i++)
            if (animation->mChannels[i]->mNodeName.data == nodeName)
                return animation->mChannels[i];
        return nullptr;
    }


    aiVector3D CalcInterpolatedValueFromKey(float animationTime, const int numKeys, const aiVectorKey* const vectorKey) const
    {

        aiVector3D ret;
        if (numKeys == 1)
        {
            ret = vectorKey[0].mValue;
            return ret;
        }

        unsigned int keyIndex = FindKeyIndex(animationTime, numKeys, vectorKey);
        unsigned int nextKeyIndex = keyIndex + 1;

        assert(nextKeyIndex < numKeys);

        float deltaTime = vectorKey[nextKeyIndex].mTime - vectorKey[keyIndex].mTime;
        float factor = (animationTime - (float)vectorKey[keyIndex].mTime) / deltaTime;

        assert(factor >= 0.0f && factor <= 1.0f);

        const aiVector3D& startValue = vectorKey[keyIndex].mValue;
        const aiVector3D& endValue = vectorKey[nextKeyIndex].mValue;

        ret.x = startValue.x + (endValue.x - startValue.x) * factor;
        ret.y = startValue.y + (endValue.y - startValue.y) * factor;
        ret.z = startValue.z + (endValue.z - startValue.z) * factor;

        return ret;
    }

    aiQuaternion CalcInterpolatedValueFromKey(float animationTime, const int numKeys, const aiQuatKey* const quatKey) const
    {
        aiQuaternion ret;
        if (numKeys == 1)
        {
            ret = quatKey[0].mValue;
            return ret;
        }

        unsigned int keyIndex = FindKeyIndex(animationTime, numKeys, quatKey);
        unsigned int nextKeyIndex = keyIndex + 1;

        assert(nextKeyIndex < numKeys);

        float deltaTime = quatKey[nextKeyIndex].mTime - quatKey[keyIndex].mTime;
        float factor = (animationTime - (float)quatKey[keyIndex].mTime) / deltaTime;

        assert(factor >= 0.0f && factor <= 1.0f);

        const aiQuaternion& startValue = quatKey[keyIndex].mValue;
        const aiQuaternion& endValue = quatKey[nextKeyIndex].mValue;
        aiQuaternion::Interpolate(ret, startValue, endValue, factor);
        ret = ret.Normalize();

        return ret;
    }

    unsigned int FindKeyIndex(const float animationTime, const int numKeys, const aiVectorKey* const vectorKey) const
    {
        assert(numKeys > 0);
        for (int i = 0; i < numKeys - 1; i++)
            if (animationTime < (float)vectorKey[i + 1].mTime)
                return i;
        assert(0);
    }

    unsigned int FindKeyIndex(const float animationTime, const int numKeys, const aiQuatKey* const quatKey) const
    {
        assert(numKeys > 0);   
        for (int i = 0; i < numKeys - 1; i++)
            if (animationTime < (float)quatKey[i + 1].mTime)
                return i;
        assert(0);
    }
};

#endif