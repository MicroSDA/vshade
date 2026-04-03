#ifndef ENGINE_CORE_RENDER_CAMERA_CAMERA_H
#define ENGINE_CORE_RENDER_CAMERA_CAMERA_H

#define GLM_ENABLE_EXPERIMENTAL

#include <engine/core/utility/factory.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtx/rotate_vector.hpp>

namespace vshade
{
namespace render
{

enum class Projection
{
    _PERSPECTIVE_,
    _ORTHOGRAPHIC_
};

class VSHADE_API Camera : public utility::CRTPFactory<Camera>
{
    friend class utility::CRTPFactory<Camera>;

public:
    static constexpr glm::vec3 const _UP_{0.0f, 1.0f, 0.0f}; // Y is up

    struct RenderData
    {
        glm::mat4 viewProjection;
        glm::mat4 view;
        glm::mat4 projection;
        alignas(16) glm::vec3 position;
        alignas(16) glm::vec3 forward;
        float near;
        float far;
    };

public:
    virtual ~Camera()                  = default;
    Camera(Camera const&)              = delete;
    Camera(Camera&&)                   = delete;
    Camera& operator=(Camera const&) & = delete;
    Camera& operator=(Camera&&) &      = delete;

    /**
     * @brief Get the view matrix.
     * @return View matrix.
     */
    glm::mat4 getView() const
    {
        return glm::lookAt(position_, position_ + forward_, up_);
    }

    /**
     * @brief Get the projection matrix.
     * @return Projection matrix.
     */
    glm::mat4 const& getProjection() const
    {
        return perpective_;
    }

    /**
     * @brief Get the combined view-projection matrix.
     * @return View-projection matrix.
     */
    glm::mat4 getViewProjection() const
    {
        return perpective_ * getView();
    }

    /**
     * @brief Get a modifiable reference to the projection matrix.
     * @return Reference to the projection matrix.
     */
    glm::mat4& getProjection()
    {
        return perpective_;
    }

    /**
     * @brief Get a modifiable reference to the forward direction.
     * @return Reference to the forward direction vector.
     */
    glm::vec3& getForwardDirection()
    {
        return forward_;
    }

    /**
     * @brief Get a constant reference to the forward direction.
     * @return Constant reference to the forward direction vector.
     */
    glm::vec3 const& getForwardDirection() const
    {
        return forward_;
    }

    /**
     * @brief Get a modifiable reference to the up direction.
     * @return Reference to the up direction vector.
     */
    glm::vec3& getUpDirection()
    {
        return up_;
    }

    /**
     * @brief Get a constant reference to the up direction.
     * @return Constant reference to the up direction vector.
     */
    glm::vec3 const& getUpDirection() const
    {
        return up_;
    }

    /**
     * @brief Get a modifiable reference to the position.
     * @return Reference to the position vector.
     */
    glm::vec3& getPosition()
    {
        return position_;
    }

    /**
     * @brief Get a constant reference to the position.
     * @return Constant reference to the position vector.
     */
    glm::vec3 const& getPosition() const
    {
        return position_;
    }

    /**
     * @brief Set the camera view using a view matrix.
     * @param view The view matrix to set.
     */
    void setVeiw(glm::mat4 const& view)
    {
        glm::mat4 matrix{getProjection() * glm::inverse(view)};
        // glm::vec2 right	= glm::vec3{matrix[0][0], matrix[1][0], matrix[2][0]};
        forward_  = glm::vec3{matrix[0][2], matrix[1][2], matrix[2][2]};
        up_       = glm::vec3{matrix[0][1], matrix[1][1], matrix[2][1]};
        position_ = glm::vec3{view[3][0], view[3][1], view[3][2]};
    }

    /**
     * @brief Get the field of view (FOV).
     * @return FOV in degrees.
     */
    float const& getFov() const
    {
        return fov_;
    }

    /**
     * @brief Get a modifiable reference to the field of view (FOV).
     * @return Reference to the FOV in degrees.
     */
    float& getFov()
    {
        return fov_;
    }

    /**
     * @brief Get the aspect ratio.
     * @return Aspect ratio.
     */
    float const& getAspect() const
    {
        return aspect_;
    }

    /**
     * @brief Get a modifiable reference to the aspect ratio.
     * @return Reference to the aspect ratio.
     */
    float& getAspect()
    {
        return aspect_;
    }

    /**
     * @brief Get the near clipping plane distance.
     * @return Near clipping plane distance.
     */
    float const& getNear() const
    {
        return z_near_;
    }

    /**
     * @brief Get a modifiable reference to the near clipping plane distance.
     * @return Reference to the near clipping plane distance.
     */
    float& getNear()
    {
        return z_near_;
    }

    /**
     * @brief Get the far clipping plane distance.
     * @return Far clipping plane distance.
     */
    float const& getFar() const
    {
        return z_far_;
    }

    /**
     * @brief Get a modifiable reference to the far clipping plane distance.
     * @return Reference to the far clipping plane distance.
     */
    float& getFar()
    {
        return z_far_;
    }

    /**
     * @brief Set the camera position.
     * @param x X coordinate of the position.
     * @param y Y coordinate of the position.
     * @param z Z coordinate of the position.
     */
    void setPosition(float x, float y, float z)
    {
        position_.x = x;
        position_.y = y;
        position_.z = z;
    }

    /**
     * @brief Set the camera position.
     * @param position The new position of the camera.
     */
    void setPosition(glm::vec3 const& position)
    {
        position_ = position;
    }

    /**
     * @brief Set the forward direction of the camera.
     * @param x X component of the forward direction.
     * @param y Y component of the forward direction.
     * @param z Z component of the forward direction.
     */
    void setForwardDirection(float x, float y, float z)
    {
        forward_.x = x;
        forward_.y = y;
        forward_.z = z;
    }

    /**
     * @brief Set the forward direction of the camera.
     * @param direction The new forward direction.
     */
    void setForwardDirection(glm::vec3 const& direction)
    {
        forward_ = direction;
    }

    /**
     * @brief Move the camera forward by a given value.
     * @param value Distance to move forward.
     */
    void moveForward(float value)
    {
        position_ += forward_ * value;
    }

    /**
     * @brief Move the camera backward by a given value.
     * @param value Distance to move backward.
     */
    void moveBackward(float value)
    {
        position_ -= forward_ * value;
    }

    /**
     * @brief Move the camera to the right by a given value.
     * @param value Distance to move right.
     */
    void moveRight(float value)
    {
        position_ -= glm::cross(up_, forward_) * value;
    }

    /**
     * @brief Move the camera to the left by a given value.
     * @param value Distance to move left.
     */
    void moveLeft(float value)
    {
        position_ += glm::cross(up_, forward_) * value;
    }

    /**
     * @brief Set the aspect ratio of the camera.
     * @param aspect New aspect ratio.
     */
    void setAspect(float const aspect)
    {
        aspect_ = aspect;
        recalculateProjection();
    }

    /**
     * @brief Set the aspect ratio of the camera.
     * @param aspect New aspect ratio.
     */
    void setAspect(float const width, float const height)
    {
        aspect_ = width / height;
        recalculateProjection();
    }

    /**
     * @brief Set the field of view (FOV) of the camera.
     * @param fov New field of view in degrees.
     */
    void setFov(float fov)
    {
        fov_ = fov;
        recalculateProjection();
    }

    /**
     * @brief Set the near clipping plane distance.
     * @param zNear New near clipping plane distance.
     */
    void setNear(float z_near)
    {
        z_near_ = z_near;
        recalculateProjection();
    }

    /**
     * @brief Set the far clipping plane distance.
     * @param zFar New far clipping plane distance.
     */
    void setFar(float z_far)
    {
        z_far_ = z_far;
        recalculateProjection();
    }

    /**
     * @brief Rotate the camera around the yaw axis.
     * @param angle Angle in radians to rotate around the yaw axis.
     */
    void rotateYaw(float angle)
    {
        // With counterclockwise issue
        // glm::mat4 rotation = glm::rotate(angle, UP);
        // Without counterclockwise issue
        glm::mat4 rotation{glm::rotate(angle, glm::dot(_UP_, up_) < 0.f ? -_UP_ : _UP_)};
        forward_ = glm::vec3{glm::normalize(rotation * glm::vec4{forward_, 0.f})};
        up_      = glm::vec3{glm::normalize(rotation * glm::vec4{up_, 0.f})};
    }

    /**
     * @brief Set the camera ywas .
     * @param angle Angle in radians to set rotation around the yaw axis.
     */
    void setYaw(float yaw)
    {
        float pitch = getPitch();
        forward_.x  = std::cos(pitch) * std::cos(yaw);
        forward_.y  = std::sin(pitch);
        forward_.z  = std::cos(pitch) * std::sin(yaw);

        forward_ = glm::normalize(forward_);

        glm::vec3 right = glm::normalize(glm::cross(_UP_, forward_));
        up_             = glm::normalize(glm::cross(forward_, right));
    }

    /**
     * @brief Rotate the camera around the pitch axis.
     * @param angle Angle in radians to rotate around the pitch axis.
     */
    void rotatePitch(float angle)
    {
        glm::vec3 right{glm::normalize(glm::cross(up_, forward_))};
        forward_ = glm::normalize(glm::rotate(-angle, right) * glm::vec4{forward_, 0.f});
        up_      = glm::normalize(glm::cross(forward_, right));
    }

    /**
     * @brief Set the camera pith .
     * @param angle Angle in radians to set rotation around the pitch axis.
     */
    void setPitch(float pitch)
    {
        float yaw  = getYaw();
        forward_.x = std::cos(pitch) * std::cos(yaw);
        forward_.y = std::sin(pitch);
        forward_.z = std::cos(pitch) * std::sin(yaw);

        forward_ = glm::normalize(forward_);

        glm::vec3 right = glm::normalize(glm::cross(glm::vec3{0.f, 1.f, 0.f}, forward_));
        up_             = glm::normalize(glm::cross(forward_, right));
    }

    /**
     * @brief Rotate the camera around the roll axis.
     * @param angle Angle in radians to rotate around the roll axis.
     */
    void rotateRoll(float angle)
    {
        up_ = glm::normalize(glm::rotate(-angle, forward_) * glm::vec4(up_, 0.f));
    }

    /**
     * @brief Set the camera roll .
     * @param angle Angle in radians to set rotation around the roll axis.
     */
    void setRoll(float roll)
    {
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), roll, forward_);
        glm::vec3 right    = glm::normalize(glm::cross(glm::vec3{0.f, 1.f, 0.f}, forward_));
        up_                = glm::normalize(glm::vec3(rotation * glm::vec4(up_, 0.0f)));
    }

    /**
     * @brief Rotate the camera around all three axes.
     * @param x Angle in radians to rotate around the yaw axis.
     * @param y Angle in radians to rotate around the pitch axis.
     * @param z Angle in radians to rotate around the roll axis.
     */
    void rotate(float x, float y, float z)
    {
        rotateYaw(-x);
        rotatePitch(-y);
        rotateRoll(z);
    }

    /**
     * @brief Rotate the camera around all three axes.
     * @param angle Vector containing angles in radians for yaw (x), pitch (y), and roll (z).
     */
    void rotate(glm::vec3 const& angle)
    {
        rotateYaw(-angle.x);
        rotatePitch(-angle.y);
        rotateRoll(angle.z);
    }

    /**
     * @brief Resize the camera perspective.
     * @param aspect New aspect ratio. If zero, keep the current aspect ratio.
     */
    void resize(float aspect = 0.f)
    {
        (aspect) ? aspect_ = aspect, recalculateProjection() : recalculateProjection();
    }

    /**
     * @brief Recalculate the camera perspective matrix.
     */
    void recalculateProjection()
    {
        if (projection_ == Projection::_PERSPECTIVE_)
        {
            perpective_ = glm::perspective(glm::radians(fov_), aspect_, z_near_, z_far_);
        }
        else if (projection_ == Projection::_ORTHOGRAPHIC_)
        {
            float half_height{fov_ * 0.5f};
            float half_width{half_height * aspect_};
            perpective_ = glm::ortho(-half_width, half_width, -half_height, half_height, -z_far_, z_far_);
        }

        perpective_[1][1] *= -1.0f; // Flip Y for Vulkan
    }

    /**
     * @brief Get the yaw angle of the camera.
     * @return Yaw angle in radians.
     */
    float getYaw() const
    {
        return glm::atan(forward_.z, forward_.x);
    }

    /**
     * @brief Get the pitch angle of the camera.
     * @return pitch angle in radians.
     */
    float getPitch() const
    {
        return glm::asin(forward_.y);
    }

    /**
     * @brief Get the pitch angle of the camera.
     * @return roll angle in radians.
     */
    float getRoll() const
    {
        return glm::atan(up_.x, up_.y);
    }

    /**
     * @brief Get camera render data.
     * @return RenderData structure containing camera data.
     */
    RenderData getRenderData() const
    {
        return {getViewProjection(), getView(), getProjection(), getPosition(), getForwardDirection(), /*-m_Perpective[3][2], m_Perpective[2][2]*/
                getNear(),           getFar()};
    }

    void setProjection(Projection const projection)
    {
        projection_ = projection;
    }

protected:
    explicit Camera(Projection const projection = Projection::_PERSPECTIVE_) : projection_{projection}
    {
        setPitch(glm::radians(-5.f));
        recalculateProjection();
    }
    /**
     * @brief Constructor for Camera.
     * @param position Initial position of the camera.
     * @param fov Field of view in degrees.
     * @param aspect Aspect ratio of the camera.
     * @param zNear Near clipping plane distance.
     * @param zFar Far clipping plane distance.
     */
    explicit Camera(glm::vec3 const& position, float fov, float aspect, float z_near, float z_far,
                    Projection const projection = Projection::_PERSPECTIVE_)
        : projection_{projection}, position_{position}, fov_{fov}, aspect_{aspect}, z_near_{z_near}, z_far_{z_far}
    {
        setPitch(glm::radians(-5.f));
        recalculateProjection();
    }

private:
    glm::mat4  perpective_{glm::identity<glm::mat4>()};
    glm::vec3  position_{0.f, 1.f, -2.f}, forward_{0.f, 0.f, 1.f}, up_{0.f, 1.f, 0.f};
    float      fov_{45.0f}, aspect_{1.f}, z_near_{0.1f}, z_far_{300.f};
    Projection projection_{Projection::_PERSPECTIVE_};

private:
};

static constexpr std::size_t _CameraDataSize_{sizeof(Camera::RenderData)};

} // namespace render
} // namespace vshade

#endif // ENGINE_CORE_RENDER_CAMERA_CAMERA_H