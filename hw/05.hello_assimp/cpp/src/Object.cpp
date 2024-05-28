#include "Object.h"

void Object::init_buffer_objects() {
    // TODO
    std::vector<float> vertices;
    std::vector<float> colors;

    for (int i = 0; i < pmesh_->mNumVertices; ++i) {
        vertices.push_back(pmesh_->mVertices[i].x);
        vertices.push_back(pmesh_->mVertices[i].y);
        vertices.push_back(pmesh_->mVertices[i].z);
        colors.push_back(pmesh_->mColors[0][i].r);
        colors.push_back(pmesh_->mColors[0][i].g);
        colors.push_back(pmesh_->mColors[0][i].b);
    }
    // VBO
    glGenBuffers(1, &position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    if (pmesh_->mColors[0] != NULL) {
        glGenBuffers(1, &color_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * colors.size(), colors.data(), GL_STATIC_DRAW);
    }

    for (unsigned int i = 0; i < pmesh_->mNumFaces; i++) {
        aiFace face = pmesh_->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // IBO
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);
}


glm::mat4 Object::get_model_matrix() const
{
    glm::mat4 mat_model = glm::mat4(1.0f);
    
    // TODO
    mat_model = mat_model * glm::translate(glm::mat4(1.0f), get_translate());
    mat_model = mat_model * glm::mat4_cast(quat_rotate_);
    mat_model = mat_model * glm::scale(glm::mat4(1.0f), get_scale());

    return mat_model;
}

void Object::draw(int loc_a_position, int loc_a_color)
{
  // TODO
  glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
  glEnableVertexAttribArray(loc_a_position);
  glVertexAttribPointer(loc_a_position, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
  glEnableVertexAttribArray(loc_a_color);
  glVertexAttribPointer(loc_a_color, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);

  glDisableVertexAttribArray(loc_a_position);
  glDisableVertexAttribArray(loc_a_color);
}
    
void Object::print_info()
{
    std::cout << "print mesh info" << std::endl;

    std::cout << "num vertices " << pmesh_->mNumVertices << std::endl;
    for (unsigned int i = 0; i < pmesh_->mNumVertices; ++i)
    {
        aiVector3D vertex = pmesh_->mVertices[i];
        std::cout << "  vertex  (" << vertex.x << ", " << vertex.y << ", " << vertex.z << ")" << std::endl;

        if (pmesh_->mColors[0] != NULL)
        {
            aiColor4D color = pmesh_->mColors[0][i];
            std::cout << "  color  (" << color.r << ", " << color.g << ", " << color.b << ", " << color.a << ")" << std::endl;
        }
    }
}
