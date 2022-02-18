#ifndef MESH_MULTI_H
#define MESH_MULTI_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;

struct Vertex_multi {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;

};


struct Material {
    //Material color lighting
    glm::vec4 Ka;
    //Diffuse reflection
    glm::vec4 Kd;
    //Mirror reflection
    glm::vec4 Ks;
};

struct Texture_multi {
    unsigned int id;
    string type;
    string path;
};

class Mesh_multi {
public:
    // mesh Data
    vector<Vertex_multi>       vertices;
    vector<unsigned int> indices;
    vector<Texture_multi>      textures;
    Material mats;
    unsigned int VAO;
    unsigned int uniformBlockIndex;

    // constructor
    Mesh_multi(vector<Vertex_multi> vertices, vector<unsigned int> indices, vector<Texture_multi> textures, Material mat)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        this->mats = mat;

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    // render the mesh
    void Draw(Shader& shader)
    {
        // bind appropriate textures
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        bool binddiffuse = true;
        bool bindspecular = true;
        bool bindnormal = true;
        bool bindambient = true;
        shader.setFloat("useDiffuseMap", 0.0f);
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
            string samplername;
            string name = textures[i].type;
            if (name == "diffuse" && binddiffuse){
                samplername = "material.diffuseSampler";
                glUniform1i(glGetUniformLocation(shader.ID, (samplername).c_str()), i);
                glBindTexture(GL_TEXTURE_2D, textures[i].id);
                binddiffuse = false;
                shader.setFloat("useDiffuseMap", 1.0f);
            }
            else if (name == "specular" && bindspecular){
                samplername = "material.specularSampler";
                glUniform1i(glGetUniformLocation(shader.ID, (samplername).c_str()), i);
                glBindTexture(GL_TEXTURE_2D, textures[i].id);
                bindspecular = false;
            }
            else if (name == "normal" && bindnormal){
                samplername = "material.normalSampler";
                glUniform1i(glGetUniformLocation(shader.ID, (samplername).c_str()), i);
                glBindTexture(GL_TEXTURE_2D, textures[i].id);
                bindnormal = false;
            }
        }
        // draw mesh

        
        glBindVertexArray(VAO);
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, uniformBlockIndex, 0, sizeof(Material));
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        if (bindspecular == true)
            shader.setFloat("useSpecularMap", 0.0f);
        else if (bindspecular == false)
            shader.setFloat("useSpecularMap", 1.0f);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
private:
    // render data 
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glGenBuffers(1, &uniformBlockIndex);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        //glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex_multi), &vertices[0], GL_STATIC_DRAW);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex_multi) + sizeof(mats), &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBlockIndex);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(mats), (void*)(&mats), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_multi), (void*)0);
        // vertex texture coords
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_multi), (void*)offsetof(Vertex_multi, TexCoords));
        // vertex normals
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_multi), (void*)offsetof(Vertex_multi, Normal));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_multi), (void*)offsetof(Vertex_multi, Tangent));

        glBindVertexArray(0);
    }
};
#endif