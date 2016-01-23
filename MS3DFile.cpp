/*
 *	LXSR Import ms3d
 * 	adapted from source at milkshape website by E 
 *	MS3DFile.cpp
 *
 *  Created on: Jun 3, 2012
 */
#pragma warning(disable : 4786)
#include "MS3DFile.h"
#include "MS3DFileI.h"
#include "Textures.h"
#include "shaders.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <set>
#include <vector>
#include <iterator>
#include <GL/glut.h>
#include <GLES3/gl3.h>

#define MAKEDWORD(a, b)      ((unsigned int)(((word)(a)) | ((word)((word)(b))) << 16))

CMS3DFile::CMS3DFile()
{
	_i = new CMS3DFileI();
	_overrideAmbient = false;
	_overrideEmissive = false;
	_overrideDiffuse = false;
	_overrideSpecular = false;

	_white = new float [4];
	_black = new float [4];

	for(int i=0;i<4;i++)
		_white[i] = 1.0;
	for(int i=0;i<3;i++)
		_black[i] = 0.0;
	_black[3] = 1.0;

}

CMS3DFile::~CMS3DFile()
{
	delete _i;
	delete _white;
	delete _black;
}

bool CMS3DFile::LoadFromFile(const char* lpszFileName)
{
	FILE *fp = fopen(lpszFileName, "rb");
	if (!fp)
		return false;

	size_t i;
	ms3d_header_t header;
	fread(&header, 1, sizeof(ms3d_header_t), fp);

	if (strncmp(header.id, "MS3D000000", 10) != 0)
		return false;

	if (header.version != 4)
		return false;

	// vertices
	word nNumVertices;
	fread(&nNumVertices, 1, sizeof(word), fp);
	_i->arrVertices.resize(nNumVertices);
	fread(&_i->arrVertices[0], nNumVertices, sizeof(ms3d_vertex_t), fp);

	// triangles
	word nNumTriangles;
	fread(&nNumTriangles, 1, sizeof(word), fp);
	_i->arrTriangles.resize(nNumTriangles);
	fread(&_i->arrTriangles[0], nNumTriangles, sizeof(ms3d_triangle_t), fp);

	// edges
	std::set<unsigned int> setEdgePair;
	for (i = 0; i < _i->arrTriangles.size(); i++)
	{
		word a, b;
		a = _i->arrTriangles[i].vertexIndices[0];
		b = _i->arrTriangles[i].vertexIndices[1];
		if (a > b)
			std::swap(a, b);
		if (setEdgePair.find(MAKEDWORD(a, b)) == setEdgePair.end())
			setEdgePair.insert(MAKEDWORD(a, b));

		a = _i->arrTriangles[i].vertexIndices[1];
		b = _i->arrTriangles[i].vertexIndices[2];
		if (a > b)
			std::swap(a, b);
		if (setEdgePair.find(MAKEDWORD(a, b)) == setEdgePair.end())
			setEdgePair.insert(MAKEDWORD(a, b));

		a = _i->arrTriangles[i].vertexIndices[2];
		b = _i->arrTriangles[i].vertexIndices[0];
		if (a > b)
			std::swap(a, b);
		if (setEdgePair.find(MAKEDWORD(a, b)) == setEdgePair.end())
			setEdgePair.insert(MAKEDWORD(a, b));
	}

	for(std::set<unsigned int>::iterator it = setEdgePair.begin(); it != setEdgePair.end(); ++it)
	{
		unsigned int EdgePair = *it;
		ms3d_edge_t Edge;
		Edge.edgeIndices[0] = (word) EdgePair;
		Edge.edgeIndices[1] = (word) ((EdgePair >> 16) & 0xFFFF);
		_i->arrEdges.push_back(Edge);
	}

	// groups
	word nNumGroups;
	fread(&nNumGroups, 1, sizeof(word), fp);
	_i->arrGroups.resize(nNumGroups);
	for (i = 0; i < nNumGroups; i++)
	{
		fread(&_i->arrGroups[i].flags, 1, sizeof(byte), fp);
		fread(&_i->arrGroups[i].name, 32, sizeof(char), fp);
		fread(&_i->arrGroups[i].numtriangles, 1, sizeof(word), fp);
		_i->arrGroups[i].triangleIndices = new word[_i->arrGroups[i].numtriangles];
		fread(_i->arrGroups[i].triangleIndices, _i->arrGroups[i].numtriangles, sizeof(word), fp);
		fread(&_i->arrGroups[i].materialIndex, 1, sizeof(char), fp);
	}

	// materials
	word nNumMaterials;
	fread(&nNumMaterials, 1, sizeof(word), fp);
	_i->arrMaterials.resize(nNumMaterials);
	fread(&_i->arrMaterials[0], nNumMaterials, sizeof(ms3d_material_t), fp);

	fread(&_i->fAnimationFPS, 1, sizeof(float), fp);
	fread(&_i->fCurrentTime, 1, sizeof(float), fp);
	fread(&_i->iTotalFrames, 1, sizeof(int), fp);

	// joints
	word nNumJoints;
	fread(&nNumJoints, 1, sizeof(word), fp);
	_i->arrJoints.resize(nNumJoints);
	for (i = 0; i < nNumJoints; i++)
	{
		fread(&_i->arrJoints[i].flags, 1, sizeof(byte), fp);
		fread(&_i->arrJoints[i].name, 32, sizeof(char), fp);
		fread(&_i->arrJoints[i].parentName, 32, sizeof(char), fp);
		fread(&_i->arrJoints[i].rotation, 3, sizeof(float), fp);
		fread(&_i->arrJoints[i].position, 3, sizeof(float), fp);
		fread(&_i->arrJoints[i].numKeyFramesRot, 1, sizeof(word), fp);
		fread(&_i->arrJoints[i].numKeyFramesTrans, 1, sizeof(word), fp);
		_i->arrJoints[i].keyFramesRot = new ms3d_keyframe_rot_t[_i->arrJoints[i].numKeyFramesRot];
		_i->arrJoints[i].keyFramesTrans = new ms3d_keyframe_pos_t[_i->arrJoints[i].numKeyFramesTrans];
		fread(_i->arrJoints[i].keyFramesRot, _i->arrJoints[i].numKeyFramesRot, sizeof(ms3d_keyframe_rot_t), fp);
		fread(_i->arrJoints[i].keyFramesTrans, _i->arrJoints[i].numKeyFramesTrans, sizeof(ms3d_keyframe_pos_t), fp);
	}

	fclose(fp);


	//Finally we must read all the textures and save their opengl ids
	_i->arrTextures.resize(nNumMaterials);//One texture per material (if no texture is used id=0)
	std::string folderPath(lpszFileName);
        std::string texturePath("./");
        folderPath = folderPath.substr(0, folderPath.find_last_of("/")+1);


	for(int i=0; i<nNumMaterials; i++){
		if(!strlen(_i->arrMaterials[i].texture) > 0){
			_i->arrTextures[i] = 0;
		}else{
			texturePath.assign(folderPath);
			texturePath.append(_i->arrMaterials[i].texture);
			_i->arrTextures[i] = LoadGLTexture(texturePath.c_str());
		}
	}
	
	return true;
}

bool CMS3DFile::SaveToFile(const char* lpszFileName)
{
	FILE *fp = fopen(lpszFileName, "w");
	if (!fp)
		return false;

	//header
	ms3d_header_t header;
	header.id[0] = 'M';
	header.id[1] = 'S';
	header.id[2] = '3';
	header.id[3] = 'D';
	header.id[4] = '0';
	header.id[5] = '0';
	header.id[6] = '0';
	header.id[7] = '0';
	header.id[8] = '0';
	header.id[9] = '0';
	header.version = 4;
	fwrite(&header, sizeof(ms3d_header_t), 1, fp);

	// vertices
	word nNumVertices = _i->arrVertices.size();
	fwrite(&nNumVertices, sizeof(word), 1, fp);
	fwrite(&_i->arrVertices[0], sizeof(ms3d_vertex_t), nNumVertices, fp);

	
	// triangles
	word nNumTriangles = _i->arrTriangles.size();
	fwrite(&nNumTriangles, sizeof(word), 1, fp);
	fwrite(&_i->arrTriangles[0], sizeof(ms3d_triangle_t), nNumTriangles, fp);	
	
	
	// groups
	word nNumGroups = _i->arrGroups.size();
	fwrite(&nNumGroups, sizeof(word), 1, fp);

	word i;
	for (i = 0; i < nNumGroups; i++)
	{
		fwrite(&_i->arrGroups[i].flags, sizeof(byte), 1, fp);
		fwrite(&_i->arrGroups[i].name, sizeof(char), 32, fp);
		fwrite(&_i->arrGroups[i].numtriangles, sizeof(word), 1, fp);
		fwrite(_i->arrGroups[i].triangleIndices, sizeof(word), _i->arrGroups[i].numtriangles, fp);
		fwrite(&_i->arrGroups[i].materialIndex, sizeof(char), 1, fp);
	}

	
	// materials
	word nNumMaterials = _i->arrMaterials.size();
	fwrite(&nNumMaterials, sizeof(word), 1, fp);
	fwrite(&_i->arrMaterials[0], sizeof(ms3d_material_t), nNumMaterials, fp);

	
	fwrite(&_i->fAnimationFPS, sizeof(float), 1, fp);
	fwrite(&_i->fCurrentTime, sizeof(float), 1, fp);
	fwrite(&_i->iTotalFrames, sizeof(int), 1, fp);
	

	// joints
	word nNumJoints = _i->arrJoints.size();
	fwrite(&nNumJoints, sizeof(word), 1, fp);
	for (i = 0; i < nNumJoints; i++)
	{
		fwrite(&_i->arrJoints[i].flags, sizeof(byte), 1, fp);
		fwrite(&_i->arrJoints[i].name, sizeof(char), 32, fp);
		fwrite(&_i->arrJoints[i].parentName, sizeof(char), 32, fp);
		fwrite(&_i->arrJoints[i].rotation, sizeof(float), 3, fp);
		fwrite(&_i->arrJoints[i].position, sizeof(float), 3, fp);
		fwrite(&_i->arrJoints[i].numKeyFramesRot, sizeof(word), 1, fp);
		fwrite(&_i->arrJoints[i].numKeyFramesTrans, sizeof(word), 1, fp);
		fwrite(_i->arrJoints[i].keyFramesRot, sizeof(ms3d_keyframe_rot_t), _i->arrJoints[i].numKeyFramesRot, fp);
		fwrite(_i->arrJoints[i].keyFramesTrans, sizeof(ms3d_keyframe_pos_t), _i->arrJoints[i].numKeyFramesTrans, fp);
	}

	fclose(fp);
	return true;
}

void CMS3DFile::Clear()
{
	_i->arrVertices.clear();
	_i->arrTriangles.clear();
	_i->arrEdges.clear();
	_i->arrGroups.clear();
	_i->arrMaterials.clear();
	_i->arrJoints.clear();
	_i->arrTextures.clear();
}

int CMS3DFile::GetNumVertices()
{
	return (int) _i->arrVertices.size();
}

void CMS3DFile::GetVertexAt(int nIndex, ms3d_vertex_t **ppVertex)
{
	if (nIndex >= 0 && nIndex < (int) _i->arrVertices.size())
		*ppVertex = &_i->arrVertices[nIndex];
}

int CMS3DFile::GetNumTriangles()
{
	return (int) _i->arrTriangles.size();
}

void CMS3DFile::GetTriangleAt(int nIndex, ms3d_triangle_t **ppTriangle)
{
	if (nIndex >= 0 && nIndex < (int) _i->arrTriangles.size())
		*ppTriangle = &_i->arrTriangles[nIndex];
}

int CMS3DFile::GetNumEdges()
{
	return (int) _i->arrEdges.size();
}

void CMS3DFile::GetEdgeAt(int nIndex, ms3d_edge_t **ppEdge)
{
	if (nIndex >= 0 && nIndex < (int) _i->arrEdges.size())
		*ppEdge = &_i->arrEdges[nIndex];
}

int CMS3DFile::GetNumGroups()
{
	return (int) _i->arrGroups.size();
}

void CMS3DFile::GetGroupAt(int nIndex, ms3d_group_t **ppGroup)
{
	if (nIndex >= 0 && nIndex < (int) _i->arrGroups.size())
		*ppGroup = &_i->arrGroups[nIndex];
}

int CMS3DFile::FindGroupByName(const char* lpszName)
{
	for (size_t i = 0; i < _i->arrGroups.size(); i++)
		if (!strcmp(_i->arrGroups[i].name, lpszName))
			return i;
	return -1;
}
int CMS3DFile::GetNumMaterials()
{
	return (int) _i->arrMaterials.size();
}

void CMS3DFile::GetMaterialAt(int nIndex, ms3d_material_t **ppMaterial)
{
	if (nIndex >= 0 && nIndex < (int) _i->arrMaterials.size())
		*ppMaterial = &_i->arrMaterials[nIndex];
}

int CMS3DFile::GetNumJoints()
{
	return (int) _i->arrJoints.size();
}

void CMS3DFile::GetJointAt(int nIndex, ms3d_joint_t **ppJoint)
{
	if (nIndex >= 0 && nIndex < (int) _i->arrJoints.size())
		*ppJoint = &_i->arrJoints[nIndex];
}

int CMS3DFile::FindJointByName(const char* lpszName)
{
	for (size_t i = 0; i < _i->arrJoints.size(); i++)
	{
		if (!strcmp(_i->arrJoints[i].name, lpszName))
			return i;
	}

	return -1;
}

float CMS3DFile::GetAnimationFPS()
{
	return _i->fAnimationFPS;
}

float CMS3DFile::GetCurrentTime()
{
	return _i->fCurrentTime;
}

int CMS3DFile::GetTotalFrames()
{
	return _i->iTotalFrames;
}

void CMS3DFile::draw(){
	GLboolean texEnabled = glIsEnabled( GL_TEXTURE_2D );
	
	for(int i=0; i < _i->arrGroups.size(); i++){
		int materialIndex = (int)_i->arrGroups[i].materialIndex;
		if( materialIndex >= 0 ){
			setMaterial(&(_i->arrMaterials[materialIndex]), materialIndex); 
		}else
			glDisable( GL_TEXTURE_2D );
		drawGroup(&(_i->arrGroups[i]));
	}		

	if ( texEnabled )
		glEnable( GL_TEXTURE_2D );
	else
		glDisable( GL_TEXTURE_2D );
}

void CMS3DFile::drawES(){
	GLboolean texEnabled = glIsEnabled( GL_TEXTURE_2D );
	
	for(int i=0; i < _i->arrGroups.size(); i++){
		int materialIndex = (int)_i->arrGroups[i].materialIndex;
		if( materialIndex >= 0 )
			setMaterial(&(_i->arrMaterials[materialIndex]), materialIndex); 
		else
			glDisable( GL_TEXTURE_2D );
		glBindVertexArray(_vao[i]);
		glDrawElements(GL_TRIANGLES, _i->arrGroups[i].numtriangles*3, GL_UNSIGNED_INT, 0);
	}		

	if ( texEnabled )
		glEnable( GL_TEXTURE_2D );
	else
		glDisable( GL_TEXTURE_2D );
	/*glBindVertexArray(_vao[0]);
	glDrawElements(GL_TRIANGLES, _i->arrTriangles.size(), GL_UNSIGNED_INT, 0);*/
}


void CMS3DFile::setMaterial(ms3d_material_t* material, int textureIndex){
	if(_overrideAmbient)
		glMaterialfv( GL_FRONT, GL_AMBIENT, _white);
	else
		glMaterialfv( GL_FRONT, GL_AMBIENT, material->ambient);
	if(_overrideDiffuse)
		glMaterialfv( GL_FRONT, GL_DIFFUSE, _white);
	else
		glMaterialfv( GL_FRONT, GL_DIFFUSE, material->diffuse );
	if(_overrideSpecular)
		glMaterialfv( GL_FRONT, GL_SPECULAR, _white );
	else
		glMaterialfv( GL_FRONT, GL_SPECULAR, material->specular );
	if(_overrideEmissive)
		glMaterialfv( GL_FRONT, GL_EMISSION, _black);
	else
		glMaterialfv( GL_FRONT, GL_EMISSION, material->emissive );

	glMaterialf( GL_FRONT, GL_SHININESS, material->shininess );
	if( _i->arrTextures[textureIndex] > 0){
		glBindTexture( GL_TEXTURE_2D, _i->arrTextures[textureIndex]);
		glEnable( GL_TEXTURE_2D );
	}else
		glDisable( GL_TEXTURE_2D );
}

//binds an opengl texture with the material information from a given group (usually to draw that same group)
void CMS3DFile::setMaterial(int texture, ms3d_group_t* group){
	ms3d_material_t* material = &(_i->arrMaterials[group->materialIndex]);	
	if(_overrideAmbient)
		glMaterialfv( GL_FRONT, GL_AMBIENT, _white);
	else
		glMaterialfv( GL_FRONT, GL_AMBIENT, material->ambient);
	if(_overrideDiffuse)
		glMaterialfv( GL_FRONT, GL_DIFFUSE, _white);
	else
		glMaterialfv( GL_FRONT, GL_DIFFUSE, material->diffuse );
	if(_overrideSpecular)
		glMaterialfv( GL_FRONT, GL_SPECULAR, _white );
	else
		glMaterialfv( GL_FRONT, GL_SPECULAR, material->specular );
	if(_overrideEmissive)
		glMaterialfv( GL_FRONT, GL_EMISSION, _black);
	else
		glMaterialfv( GL_FRONT, GL_EMISSION, material->emissive );

	glMaterialf( GL_FRONT, GL_SHININESS, material->shininess );
	glBindTexture( GL_TEXTURE_2D, texture);
	glEnable( GL_TEXTURE_2D );
}

void CMS3DFile::drawGroup(ms3d_group_t* group){
	glBegin( GL_TRIANGLES );{
		int numTriangles = group->numtriangles;
		for(int j=0; j<numTriangles; j++){
			int triangleIndex = (int)group->triangleIndices[j];
			ms3d_triangle_t* tri = &(_i->arrTriangles[triangleIndex]);
			//Draw each vertex
			//for(int k=0;k<3;k++){
				//Aparently gcc would still do the for loop even with -O3 so we just expand it by hand
				glNormal3fv( tri->vertexNormals[0] );
				glTexCoord2f( tri->s[0], tri->t[0] );
				glVertex3fv( _i->arrVertices[tri->vertexIndices[0]].vertex );
			
				glNormal3fv( tri->vertexNormals[1] );
				glTexCoord2f( tri->s[1], tri->t[1] );
				glVertex3fv( _i->arrVertices[tri->vertexIndices[1]].vertex );
				
				glNormal3fv( tri->vertexNormals[2] );
				glTexCoord2f( tri->s[2], tri->t[2] );
				glVertex3fv( _i->arrVertices[tri->vertexIndices[2]].vertex );
			//}
		}
	}glEnd();
}

void CMS3DFile::prepareModel(){
	
	_vao = new GLuint [_i->arrGroups.size()];

	glGenVertexArrays(_i->arrGroups.size(), _vao);

	for(int i=0; i < _i->arrGroups.size(); i++){
		prepareGroup(&(_i->arrGroups[i]), _vao[i]);
	}		

	/*glBindVertexArray(_vao[0]);

	GLuint vbo;
	glGenBuffers(1, &vbo);

	GLfloat vertices_position[_i->arrVertices.size()*3];

	int j = 0;
	for(int i = 0; i<_i->arrVertices.size(); i++){
		vertices_position[j++] = _i->arrVertices[i].vertex[0];
		vertices_position[j++] = _i->arrVertices[i].vertex[1];
		vertices_position[j++] = _i->arrVertices[i].vertex[2];
	}


	GLfloat texture_coord[_i->arrTriangles.size()*6];
	j = 0;
	for(int i = 0; i<_i->arrTriangles.size(); i++){
		texture_coord[j++] = _i->arrTriangles[i].s[0];
		texture_coord[j++] = _i->arrTriangles[i].t[0];
		texture_coord[j++] = _i->arrTriangles[i].s[1];
		texture_coord[j++] = _i->arrTriangles[i].t[1];
		texture_coord[j++] = _i->arrTriangles[i].s[2];
		texture_coord[j++] = _i->arrTriangles[i].t[2];
	}

	GLuint indices[_i->arrTriangles.size()]; 
	j = 0;
	for(int i = 0; i<_i->arrTriangles.size(); i++){
		indices[j++] = _i->arrTriangles[i].vertexIndices[0];	
		indices[j++] = _i->arrTriangles[i].vertexIndices[1];	
		indices[j++] = _i->arrTriangles[i].vertexIndices[2];
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_position) + sizeof(texture_coord), NULL, GL_STATIC_DRAW);

	//Copy the data to the buffer
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices_position) , vertices_position);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices_position), sizeof(texture_coord), texture_coord);

	//Set up the indices
	GLuint eab;
	glGenBuffers(1, &eab);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eab);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        _shaderProgram = create_program("shaders/vert.shader", "shaders/frag.shader");
	
	//Position Attribute
	GLint position_attribute = glGetAttribLocation(_shaderProgram, "position");
	glVertexAttribPointer(position_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, 0);
	glEnableVertexAttribArray(position_attribute);

	//Texture coord attribute
	GLint texture_coord_attribute = glGetAttribLocation(_shaderProgram, "texture_coord");
	glVertexAttribPointer(texture_coord_attribute, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(vertices_position));
	glEnableVertexAttribArray(texture_coord_attribute);
	*/
}

void CMS3DFile::prepareGroup(ms3d_group_t* group, GLuint vao){
	glBindVertexArray(vao);
		
	int numTriangles = group->numtriangles;

	GLfloat vertices_position[numTriangles*3*4];
	GLfloat vertices_normals[numTriangles*9];
	GLfloat texture_coord[numTriangles*6];

	int numVertices = numTriangles*3;
	GLuint indices[numVertices];
	
	//GLfloat vertices_position[_i->arrVertices.size()*3];

	//Fill the arrays
	int i = 0;
	int t = 0;
	int l = 0;
	int m = 0;
	for(int j=0; j<numTriangles; j++){
		int triangleIndex = (int)group->triangleIndices[j];
		ms3d_triangle_t* tri = &(_i->arrTriangles[triangleIndex]);
		for(int k = 0; k < 3; k++){
			vertices_position[i] = _i->arrVertices[tri->vertexIndices[k]].vertex[0]; 
			vertices_position[i+1] = _i->arrVertices[tri->vertexIndices[k]].vertex[1]; 
			vertices_position[i+2] = _i->arrVertices[tri->vertexIndices[k]].vertex[2];
			vertices_position[i+3] = 1.0;

			vertices_normals[l] = tri->vertexNormals[k][0];
			vertices_normals[l+1] = tri->vertexNormals[k][1];
			vertices_normals[l+2] = tri->vertexNormals[k][2];

			texture_coord[t++] = tri->s[k];
			texture_coord[t++] = tri->t[k];

			i += 4;
			l += 3;
		}
	}

	for(int i = 0; i<numVertices; i++){
		indices[i] = i;
	}


	GLuint vbo;
	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_position) + sizeof(texture_coord) + sizeof(vertices_normals), NULL, GL_STATIC_DRAW);

	/*Copy the data to the buffer*/
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices_position), vertices_position);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices_position), sizeof(texture_coord), texture_coord);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices_position) + sizeof(texture_coord), sizeof(vertices_normals), vertices_normals);

	//Set up the indices
	GLuint eab;
	glGenBuffers(1, &eab);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eab);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        _shaderProgram = create_program("shaders/vert.shader", "shaders/frag.shader");
	
	//Position Attribute
	GLint position_attribute = glGetAttribLocation(_shaderProgram, "position");
	glVertexAttribPointer(position_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, 0);
	glEnableVertexAttribArray(position_attribute);

	//Texture coord attribute
	GLint texture_coord_attribute = glGetAttribLocation(_shaderProgram, "texture_coord");
	glVertexAttribPointer(texture_coord_attribute, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(vertices_position));
	glEnableVertexAttribArray(texture_coord_attribute);

	//TODO Pass the normals and implement lighting in the shaders
}

void CMS3DFile::setOverrideAmbient(bool overrideAmbient){
	_overrideAmbient = overrideAmbient;
}
void CMS3DFile::setOverrideDiffuse(bool overrideDiffuse){
	_overrideDiffuse = overrideDiffuse;
}
void CMS3DFile::setOverrideSpecular(bool overrideSpecular){
	_overrideSpecular = overrideSpecular;
}
void CMS3DFile::setOverrideEmissive(bool overrideEmissive){
	_overrideEmissive = overrideEmissive;
}

GLuint CMS3DFile::getShaderProgram(){
	return _shaderProgram;
}

void CMS3DFile::optimize(){
	mergeGroups();
	removeUnusedMaterials();
}
