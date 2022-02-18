#ifndef MESH_ANI_H
#define MESH_ANI_H

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

struct Vertex_ani {
    // position
    glm::vec3 Position;
    // texCoords
    glm::vec2 TexCoords;
    // normal
    glm::vec3 Normal;
    // tangent
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
    glm::ivec4 v_boneIds;
    glm::vec4 v_boneWeights;
};

struct Texture_ani {
    unsigned int id;
    string type;
    string path;
};

class Mesh_ani{
public:
    // mesh Data
    vector<Vertex_ani>       vertices;
    vector<unsigned int> indices;
    vector<Texture_ani>      textures;
    unsigned int VAO;
    // constructor
    Mesh_ani(vector<Vertex_ani> vertices, vector<unsigned int> indices, vector<Texture_ani> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    // render the mesh
    void bind(Shader& shader)
    {
        // bind appropriate textures
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
            string number;
            string name = textures[i].type;
            if (name == "diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "specular")
                number = std::to_string(specularNr++); // transfer unsigned int to stream
            else if (name == "normal")
                number = std::to_string(normalNr++); // transfer unsigned int to stream
            else if (name == "height")
                number = std::to_string(heightNr++); // transfer unsigned int to stream

            // now set the sampler to the correct texture unit
            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            // and finally bind the texture
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
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

        //for (unsigned int i = 0; i < vertices.size(); i++)
        //    cout << vertices[i].boneIds[0] <<"dd"<< vertices[i].boneIds[0] <<"dd"<< vertices[i].boneIds[0] <<"dd"<< vertices[i].boneIds[0];

        //setupMesh();
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
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
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

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex_ani), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_ani), (void*)0);
        // vertex texture coords
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_ani), (void*)offsetof(Vertex_ani, TexCoords));
        // vertex normals
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_ani), (void*)offsetof(Vertex_ani, Normal));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_ani), (void*)offsetof(Vertex_ani, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_ani), (void*)offsetof(Vertex_ani, Bitangent));
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex_ani), (void*)offsetof(Vertex_ani, v_boneIds));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_ani), (void*)offsetof(Vertex_ani, v_boneWeights));
        glBindVertexArray(0);
    }
};
#endif