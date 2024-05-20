#include "Camera.h"

// TODO: fill up the following function properly 
void Camera::set_rotation(const glm::quat& _q)
{
  glm::mat4 rotation_matrix = glm::mat4_cast(_q);
  front_dir_ = glm::normalize(glm::vec3(rotation_matrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
  up_dir_ = glm::normalize(glm::vec3(rotation_matrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));
  right_dir_ = glm::normalize(glm::cross(front_dir_, up_dir_));
}

// TODO: re-write the following function properly 
const glm::quat Camera::get_rotation() const
{
  glm::mat3 rotation_matrix = glm::mat3(right_dir_,up_dir_,-front_dir_);
  return glm::quat_cast(rotation_matrix);
}

// TODO: fill up the following function properly 
void Camera::set_pose(const glm::quat& _q, const glm::vec3& _t)
{
  set_rotation(_q);
  position_ = _t;
}

// TODO: fill up the following function properly 
void Camera::get_pose(glm::quat& _q, glm::vec3& _t) const
{
  _q = get_rotation();
  _t = position_;
}

// TODO: rewrite the following function properly 
const glm::mat4 Camera::get_pose() const
{
  glm::mat4 translate_matrix = glm::translate(glm::mat4(1.0f), position_);
  glm::mat4 rotation_matrix = glm::mat4_cast(get_rotation());
  return translate_matrix * rotation_matrix;
}

// TODO: fill up the following function properly 
void Camera::set_pose(const glm::mat4& _frame)
{
  // +x_cam: right direction of the camera    (it should be a unit vector whose length is 1)
  // right_dir_ = ..... ; // +x
  right_dir_ = _frame[0];
  // +y_cam: up direction of the camera       (it should be a unit vector whose length is 1)   
  // up_dir_    = ..... ;    // +y
  up_dir_ = _frame[1];
  // -z_cam: front direction of the camera    (it should be a unit vector whose length is 1)
  // front_dir_ = ..... ;    // -z
  front_dir_ = _frame[2];
  // pos_cam: position of the camera
  // position_  = ..... ;    // pos
  position_ = _frame[3];
}

// TODO: fill up the following function properly 
void Camera::set_pose(const glm::vec3& _pos, const glm::vec3& _at, const glm::vec3& _up_dir)
{
  // up_dir_    = ..... ;
  // front_dir_ = ..... ;    // -z_cam direction
  front_dir_ = glm::normalize(_at - _pos);
  // right_dir_ = ..... ;    // +x_cam direction
  right_dir_ = glm::normalize(glm::cross(_up_dir, front_dir_));
  // up_dir_    = ..... ;    // +y_cam direction
  up_dir_ = glm::normalize(glm::cross(front_dir_, right_dir_));

  // position_  = ..... ;      // pos
  position_ = _pos;
}

// TODO: rewrite the following function properly 
const glm::mat4 Camera::get_view_matrix() const
{
  glm::mat4 TR_matrix = get_pose();
  return glm::inverse(TR_matrix);
}

// TODO: rewrite the following function properly 
const glm::mat4 Camera::get_projection_matrix() const
{
  // TODO: Considering the followings,
  //       You must return a proper projection matrix
  //       i) camera mode: it can be either kOrtho or kPerspective
  //       ii) zoom-in/-out: if the camera mode is kOrtho, 
  //                         utilize ortho_scale_ for zoom-in/-out
  //                         if the camera mode is kPerspective,
  //                         utilize fovy_ for zoom-in/-out
  //       iii) aspect ratio: utilize aspect_ in the both camera modes
  //       iv) near/far clipping planes: utilize near_, far_
  float half_height = ortho_scale_ * 0.5f;
  float half_width = aspect_ * half_height;

  return (mode_ == kOrtho) ?
      glm::ortho(-half_width, half_width, -half_height, half_height, near_, far_) : 
      glm::perspective(glm::radians(fovy_), aspect_, near_, far_);
}

// TODO: fill up the following functions properly 
void Camera::move_forward(float delta)
{
  position_[0] += front_dir_[0] * delta;
  position_[1] += front_dir_[1] * delta;
  position_[2] += front_dir_[2] * delta;
}

// TODO: fill up the following functions properly 
void Camera::move_backward(float delta)
{
  position_[0] += front_dir_[0] * (-delta);
  position_[1] += front_dir_[1] * (-delta);
  position_[2] += front_dir_[2] * (-delta);
}

// TODO: fill up the following functions properly 
void Camera::move_left(float delta)
{
  position_[0] += right_dir_[0] * (-delta);
  position_[1] += right_dir_[1] * (-delta);
  position_[2] += right_dir_[2] * (-delta);
}

// TODO: fill up the following functions properly 
void Camera::move_right(float delta)
{
  position_[0] += right_dir_[0] * delta;
  position_[1] += right_dir_[1] * delta;
  position_[2] += right_dir_[2] * delta;
}

// TODO: fill up the following functions properly 
void Camera::move_up(float delta)
{
  position_[0] += up_dir_[0] * delta;
  position_[1] += up_dir_[1] * delta;
  position_[2] += up_dir_[2] * delta;
}

// TODO: fill up the following functions properly 
void Camera::move_down(float delta)
{
  position_[0] += up_dir_[0] * (-delta);
  position_[1] += up_dir_[1] * (-delta);
  position_[2] += up_dir_[2] * (-delta);
}
