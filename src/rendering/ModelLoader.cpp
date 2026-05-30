#include "renderer/rendering/ModelLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <iostream>

namespace renderer::rendering {

    std::unique_ptr<Mesh> ModelLoader::load(const std::string& path)
    {
        Assimp::Importer importer;

        const aiScene* scene =
            importer.ReadFile(
                path,
                aiProcess_Triangulate |
                aiProcess_GenNormals |
                aiProcess_FlipUVs
            );

        if (!scene || !scene->HasMeshes()) {
            std::cerr << importer.GetErrorString() << std::endl;
            return nullptr;
        }

        aiMesh* mesh = scene->mMeshes[0];

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        vertices.reserve(mesh->mNumVertices);

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {

            Vertex v;

            v.position = {
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            };

            if (mesh->HasNormals()) {
                v.normal = {
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z
                };
            }

            if (mesh->HasTextureCoords(0)) {
                v.texCoord = {
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                };
            }

            vertices.push_back(v);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {

            aiFace& face = mesh->mFaces[i];

            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        return std::make_unique<Mesh>(vertices, indices);
    }

}