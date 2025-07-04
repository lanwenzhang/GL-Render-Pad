#include "assimp_instance_loader.h"
#include "../tools/tools.h"
#include "../renderer/scene/mesh/material/g_buffer_material.h"

namespace lzgl::loader {

	lzgl::renderer::Object* AssimpInstanceLoader::load(const std::string& path, int instanceCount) {

		std::size_t lastIndex = path.find_last_of("//");
		auto rootPath = path.substr(0, lastIndex + 1);

		lzgl::renderer::Object* rootNode = new lzgl::renderer::Object();

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {

			std::cout << "Error: Model Read Failed" << std::endl;
			return nullptr;
		}

		processNode(scene->mRootNode, rootNode, scene, rootPath, instanceCount);

		return rootNode;
	}

	void AssimpInstanceLoader::processNode(aiNode* ainode, lzgl::renderer::Object* parent, const aiScene* scene, const std::string& rootPath, int instanceCount) {

		// 1 Generate node and link parent
		lzgl::renderer::Object* node = new lzgl::renderer::Object();
		parent->addChild(node);

		// 2 Get node local transformation
		glm::mat4 localMatrix = getMat4f(ainode->mTransformation);
		glm::vec3 position, eulerAngle, scale;
		lzgl::tools::Tools::decompose(localMatrix, position, eulerAngle, scale);
		node->setPosition(position);
		node->setAngleX(eulerAngle.x);
		node->setAngleY(eulerAngle.y);
		node->setAngleZ(eulerAngle.z);
		node->setScale(scale);

		// 3 Check mesh
		for (int i = 0; i < ainode->mNumMeshes; i++) {

			int meshID = ainode->mMeshes[i];
			aiMesh* aimesh = scene->mMeshes[meshID];

			auto mesh = processMesh(aimesh, scene, rootPath, instanceCount);

			node->addChild(mesh);
		}

		// 4 Check child
		for (int i = 0; i < ainode->mNumChildren; i++) {

			processNode(ainode->mChildren[i], node, scene, rootPath, instanceCount);
		}
	}

	lzgl::renderer::InstancedMesh* AssimpInstanceLoader::processMesh(aiMesh* aimesh, const aiScene* scene, const std::string& rootPath, int instanceCount) {

		std::vector<float> positions;
		std::vector<float> normals;
		std::vector<float> uvs;
		std::vector<unsigned int> indices;
		std::vector<float> tangents;

		// 1 Positions information
		for (int i = 0; i < aimesh->mNumVertices; i++) {

			positions.push_back(aimesh->mVertices[i].x);
			positions.push_back(aimesh->mVertices[i].y);
			positions.push_back(aimesh->mVertices[i].z);

			normals.push_back(aimesh->mNormals[i].x);
			normals.push_back(aimesh->mNormals[i].y);
			normals.push_back(aimesh->mNormals[i].z);


			// number 0 uvs are texture uv
			if (aimesh->mTextureCoords[0]) {

				uvs.push_back(aimesh->mTextureCoords[0][i].x);
				uvs.push_back(aimesh->mTextureCoords[0][i].y);
			}
			else {
				uvs.push_back(0.0f);
				uvs.push_back(0.0f);
			}
			if (aimesh->HasTangentsAndBitangents()) {

				tangents.push_back(aimesh->mTangents[i].x);
				tangents.push_back(aimesh->mTangents[i].y);
				tangents.push_back(aimesh->mTangents[i].z);
			}

		}


		// 2 Indices information
		for (int i = 0; i < aimesh->mNumFaces; i++) {

			aiFace face = aimesh->mFaces[i];

			for (int j = 0; j < face.mNumIndices; j++) {

				indices.push_back(face.mIndices[j]);

			}
		}

		// 3 Create geometry
		auto geometry = new lzgl::renderer::Geometry(positions, normals, uvs, tangents, indices);
		auto material = new lzgl::renderer::GBufferMaterial();
		material->mDepthWrite = false;

		// 4 Create texture
		if (aimesh->mMaterialIndex >= 0) {
			lzgl::wrapper::Texture* texture = nullptr;
			aiMaterial* aiMat = scene->mMaterials[aimesh->mMaterialIndex];

			// 4.1 Read diffuse texture
			texture = processTexture(aiMat, aiTextureType_DIFFUSE, scene, rootPath);
			if (texture == nullptr) {
				texture = lzgl::wrapper::Texture::createTexture("assets/textures/defaultTexture.jpg", 0);
			}
			texture->setUnit(0);
			material->mDiffuse = texture;

			// 4.2 Read specular texture
			auto specularMask = processTexture(aiMat, aiTextureType_SPECULAR, scene, rootPath);
			if (specularMask == nullptr) {
				specularMask = lzgl::wrapper::Texture::createTexture("assets/textures/defaultTexture.jpg", 0);
			}
			specularMask->setUnit(1);
			material->mSpecularMask = specularMask;
		}
		else {
			material->mDiffuse = lzgl::wrapper::Texture::createTexture("assets/textures/defaultTexture.jpg", 0);
		}

		return new lzgl::renderer::InstancedMesh(geometry, material, instanceCount);

	}

	lzgl::wrapper::Texture* AssimpInstanceLoader::processTexture(
		const aiMaterial* aimat,
		const aiTextureType& type,
		const aiScene* scene,
		const std::string& rootPath
	) {

		lzgl::wrapper::Texture* texture = nullptr;

		aiString aipath;
		aimat->Get(AI_MATKEY_TEXTURE(type, 0), aipath);

		if (!aipath.length) {
			return nullptr;
		}

		// Check if fbx has texture
		const aiTexture* aitexture = scene->GetEmbeddedTexture(aipath.C_Str());
		if (aitexture) {

			unsigned char* dataIn = reinterpret_cast<unsigned char*>(aitexture->pcData);
			uint32_t widthIn = aitexture->mWidth;
			uint32_t heightIn = aitexture->mHeight;
			texture = lzgl::wrapper::Texture::createTextureFromMemory(aipath.C_Str(), 0, dataIn, widthIn, heightIn);
		}
		else {

			std::string fullPath = rootPath + aipath.C_Str();
			texture = lzgl::wrapper::Texture::createTexture(fullPath, 0);
		}

		return texture;
	}

	glm::mat4 AssimpInstanceLoader::getMat4f(aiMatrix4x4 value) {

		glm::mat4 to(
			value.a1, value.a2, value.a3, value.a4,
			value.b1, value.b2, value.b3, value.b4,
			value.c1, value.c2, value.c3, value.c4,
			value.d1, value.d2, value.d3, value.d4
		);

		return to;
	}


}

