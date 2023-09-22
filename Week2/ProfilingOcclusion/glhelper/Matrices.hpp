#pragma once

#include <Eigen/Dense>
#include <cmath>

Eigen::Matrix4f perspective(float fov, float aspect, float zNear, float zFar);

