/*
 * 	adapted from source at milkshape website by E 
 *	MS3DFile.cpp
 *
 *  Created on: Jun 3, 2012
 */
//#pragma warning(disable : 4786)
#include <GL/glew.h>
#include "MS3DFile.h"
#include "MS3DFileI.h"
#include <cstring>
#include <set>
#include <vector>
#include <iterator>
#include <GL/glut.h>


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
	
	for(unsigned int i=0; i < _i->arrGroups.size(); i++){
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

void CMS3DFile::drawGL3(){
	GLboolean texEnabled = glIsEnabled( GL_TEXTURE_2D );
	
	for(unsigned int i=0; i < _i->arrGroups.size(); i++){
		int materialIndex = (int)_i->arrGroups[i].materialIndex;
		if( materialIndex >= 0 )
			setMaterialGL3(&(_i->arrMaterials[materialIndex]),materialIndex); 
		else
			glDisable( GL_TEXTURE_2D );
		glBindVertexArray(_vao[i]);
		glDrawElements(GL_TRIANGLES, _i->arrGroups[i].numtriangles*3, GL_UNSIGNED_INT, 0);
	}		

	if ( texEnabled )
		glEnable( GL_TEXTURE_2D );
	else
		glDisable( GL_TEXTURE_2D );
}


void CMS3DFile::setMaterialGL3(ms3d_material_t* material, int textureIndex){
	GLint ambient = glGetUniformLocation(_shader, "ambient");
	GLint diffuse = glGetUniformLocation(_shader, "diffuse");
	GLint specular = glGetUniformLocation(_shader, "specular");
	GLint emissive = glGetUniformLocation(_shader, "emissive");
	GLint shininess = glGetUniformLocation(_shader, "shininess");
	GLint transparency = glGetUniformLocation(_shader, "transparency");
	
	glUniform4fv(ambient, 1, material->ambient);
	glUniform4fv(diffuse, 1, material->diffuse);
	glUniform4fv(specular, 1, material->specular);
	glUniform4fv(emissive, 1, material->emissive);

	glUniform1f(shininess, material->shininess);
	glUniform1f(transparency, 1-(material->transparency));
	
	if( _i->arrTextures[textureIndex] > 0){
		glBindTexture( GL_TEXTURE_2D, _i->arrTextures[textureIndex]);
		glEnable( GL_TEXTURE_2D );
	}else
		glDisable( GL_TEXTURE_2D );
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

void CMS3DFile::setTexture(unsigned int textureIndex, int texture){
	if(textureIndex < _i->arrTextures.size())
		_i->arrTextures[textureIndex] = texture;
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

void CMS3DFile::prepareModel(GLuint shader){
	_shader = shader;

	_vao = new GLuint [_i->arrGroups.size()];

	glGenVertexArrays(_i->arrGroups.size(), _vao);

	for(unsigned int i=0; i < _i->arrGroups.size(); i++){
		prepareGroup(&(_i->arrGroups[i]), _vao[i]);
	}		

}

void CMS3DFile::prepareGroup(ms3d_group_t* group, GLuint vao){
	glBindVertexArray(vao);
		
	int numTriangles = group->numtriangles;

	GLfloat vertices_position[numTriangles*3*4];
	GLfloat vertices_normals[numTriangles*9];
	GLfloat texture_coord[numTriangles*6];

	int numVertices = numTriangles*3;
	GLuint indices[numVertices];

	//Fill the arrays
	int vertex_coordinate_index = 0;
	int texture_coordinate_index = 0;
	int normal_coordinate_index = 0;
	for(int j=0; j<numTriangles; j++){
		int triangleIndex = (int)group->triangleIndices[j];
		ms3d_triangle_t* tri = &(_i->arrTriangles[triangleIndex]);
		for(int k = 0; k < 3; k++){
			vertices_position[vertex_coordinate_index++] = _i->arrVertices[tri->vertexIndices[k]].vertex[0]; 
			vertices_position[vertex_coordinate_index++] = _i->arrVertices[tri->vertexIndices[k]].vertex[1]; 
			vertices_position[vertex_coordinate_index++] = _i->arrVertices[tri->vertexIndices[k]].vertex[2];
			vertices_position[vertex_coordinate_index++] = 1.0;

			vertices_normals[normal_coordinate_index++] = tri->vertexNormals[k][0];
			vertices_normals[normal_coordinate_index++] = tri->vertexNormals[k][1];
			vertices_normals[normal_coordinate_index++] = tri->vertexNormals[k][2];

			texture_coord[texture_coordinate_index++] = tri->s[k];
			texture_coord[texture_coordinate_index++] = tri->t[k];
		}
	}

	for(int i = 0; i<numVertices; i++){
		indices[i] = i;
	}



	GLuint vbo;
	glGenBuffers(1, &vbo);

	size_t totalSize = sizeof(vertices_position) + sizeof(texture_coord) + sizeof(vertices_normals);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, totalSize, NULL, GL_STATIC_DRAW);

	/*Copy the data to the buffer*/
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices_position), vertices_position);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices_position), sizeof(texture_coord), texture_coord);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices_position) + sizeof(texture_coord), sizeof(vertices_normals), vertices_normals);

	//Set up the indices
	GLuint eab;
	glGenBuffers(1, &eab);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eab);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//Position Attribute
	GLint position_attribute = glGetAttribLocation(_shader, "position");
	glVertexAttribPointer(position_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, 0);
	glEnableVertexAttribArray(position_attribute);

	//Texture coord attribute
	GLint texture_coord_attribute = glGetAttribLocation(_shader, "texture_coord");
	glVertexAttribPointer(texture_coord_attribute, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(vertices_position));
	glEnableVertexAttribArray(texture_coord_attribute);


	//Normals attribute
	GLint normals_attribute = glGetAttribLocation(_shader, "normal");
	glVertexAttribPointer(normals_attribute, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(vertices_position)+sizeof(texture_coord)) );
	glEnableVertexAttribArray(normals_attribute);
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

void CMS3DFile::optimize(){
	mergeGroups();
	removeUnusedMaterials();
}

void CMS3DFile::createRectangle(float width, float height, GLuint texture){

	float vertBuffer[6*3] = { 0,height,0, 0,0,0, width,0,0,  width,0,0, width,height,0, 0,height,0 };
	float normal[3] = { 0,0,1 };
	float texCoordBuffer[6*2] = { 0,0, 0,1, 1,1, 1,1, 1,0, 0,0 };

	int vertIndex = 0;

	word nNumVertices = 6;
	_i->arrVertices.resize(nNumVertices);
	ms3d_vertex_t* vert;
	for(int i=0; i<nNumVertices; i++){
		vert = &_i->arrVertices[i];
		vert->flags = 0;
		vert->boneId = -1;
		vert->vertex[0] = vertBuffer[vertIndex++];
		vert->vertex[1] = vertBuffer[vertIndex++];
		vert->vertex[2] = vertBuffer[vertIndex++];
	}
	
	vertIndex = 0;
	int texIndex = 0;
	
	word nNumTriangles = 2;
	_i->arrTriangles.resize(nNumTriangles);
	ms3d_triangle_t* tri;
	for(int i=0; i<nNumTriangles; i++){
		tri = &_i->arrTriangles[i];
		tri->flags = 0;
		tri->vertexIndices[0] = vertIndex++;
		tri->vertexIndices[1] = vertIndex++;
		tri->vertexIndices[2] = vertIndex++;

		for(int j=0; j<3; j++){
			tri->vertexNormals[j][0] = normal[0];
			tri->vertexNormals[j][1] = normal[1];
			tri->vertexNormals[j][2] = normal[2];
		}
		for(int j=0; j<3; j++){
			tri->s[j] = texCoordBuffer[texIndex++];
			tri->t[j] = texCoordBuffer[texIndex++];
		}
		tri->smoothingGroup = 1;
		tri->groupIndex = 0;
	}

	word nNumGroups = 1;
	_i->arrGroups.resize(nNumGroups);
	ms3d_group_t* group = &_i->arrGroups[0];
	group->flags = 0;
	group->name[0] = '1';
	group->name[1] = '\0';
	group->numtriangles = 2;
	group->triangleIndices = new word[2];
	group->triangleIndices[0] = 0;
	group->triangleIndices[1] = 1;
	group->materialIndex = 0;

	word nNumMaterials = 1;
	_i->arrMaterials.resize(nNumMaterials);
	ms3d_material_t* mat = &_i->arrMaterials[0];
	mat->name[0] = '1';
	mat->name[1] = '\0';
	for(int i=0; i<3; i++){
		mat->ambient[i] = 25;
		mat->diffuse[i] = 200;
		mat->specular[i] = 0;
		mat->emissive[i] = 0;
	}
	mat->ambient[3] = 0;
	mat->diffuse[3] = 0;
	mat->specular[3] = 0;
	mat->emissive[3] = 0;
	mat->shininess = 0;
	mat->transparency = 1;
	mat->mode = 0;
	mat->texture[0] = '\0';
	mat->alphamap[0] = '\0';
	
	_i->arrTextures.resize(1);
	_i->arrTextures[0] = texture;

	_i->arrJoints.resize(0);
}

