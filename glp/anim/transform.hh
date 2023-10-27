#pragma once

#include "external/glm/glm.hpp"
#include "external/glm/gtc/quaternion.hpp"
#include "external/glm/gtx/quaternion.hpp"

namespace transform {
    
    constexpr float epsilon = 0.000001f;

    struct _t {
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
    
        inline _t(const glm::vec3& p, const glm::quat& r, const glm::vec3& s) : position{p}, rotation{r}, scale{s} {};
        inline _t() : position{0, 0, 0}, rotation{0, 0, 0, 1}, scale{1, 1, 1} {};
        inline ~_t() = default;
    };
    
    inline _t combine(const _t& a, const _t& b) {
        _t out {};
        out.scale = a.scale * a.rotation;
        out.rotation = b.rotation * a.rotation;
        out.position = a.rotation * (a.scale * b.position);
        out.position += a.position;
        return out;
    }

    inline _t inverse(const _t& t) {
        _t inv {};
        inv.rotation = glm::inverse(t.rotation);
        inv.scale.x = std::fabs(t.scale.x) < epsilon ? 0.0f : 1.0f / t.scale.x;
        inv.scale.y = std::fabs(t.scale.y) < epsilon ? 0.0f : 1.0f / t.scale.y;
        inv.scale.z = std::fabs(t.scale.z) < epsilon ? 0.0f : 1.0f / t.scale.z;

        glm::vec3 inv_t = t.position * -1.0f;
        inv.position = inv.rotation * (inv.scale * inv_t);
        return inv;
    }

    inline _t mix(const _t& a, const _t& b, float t) {
        glm::quat b_rot = b.rotation;
        if(glm::dot(a.rotation, b_rot) < 0.0f) b_rot = -b_rot;
        return _t{
            glm::mix(a.position, b.position, t),
            glm::lerp(a.rotation, b_rot, t),
            glm::mix(a.scale, b.scale, t)
        };
    }

    inline glm::mat4 to_mat4(const _t& t) {
        glm::vec3 x = t.rotation * glm::vec3(1, 0, 0);
        glm::vec3 y = t.rotation * glm::vec3(0, 1, 0);
        glm::vec3 z = t.rotation * glm::vec3(0, 0, 1);
        x *= t.scale.x;
        y *= t.scale.y;
        z *= t.scale.z;
        glm::vec3 p = t.position;

        return glm::mat4{
            x.x, x.y, x.z, 0,
            y.x, y.y, y.z, 0,
            z.x, z.y, z.z, 0,
            p.x, p.y, p.z, 1
        };
    }

    inline _t from_mat4(const glm::mat4& m) {
        _t out {};
        out.position = glm::vec3{m[3].x, m[3].y, m[3].z};
        out.rotation = glm::toQuat(m);

        glm::mat4 rot_scaled{
            m[0].x, m[0].y, m[0].z, 0,
            m[1].x, m[1].y, m[1].z, 0,
            m[2].x, m[2].y, m[2].z, 0,
            0, 0, 0, 1
        };

        glm::mat4 inv_rot = glm::toMat4(glm::inverse(out.rotation));
        glm::mat4 scale_skew = rot_scaled * inv_rot;

        out.scale = glm::vec3{m[0].x, m[1].y, m[2].z};
        return out;
    }

    inline glm::vec3 vec3(const _t& a, const glm::vec3& b) {
        glm::vec3 out = a.rotation * (a.scale*b);
        return out;
    }

    inline glm::vec3 point(const _t& a, const glm::vec3& b) {
        glm::vec3 out = vec3(a, b);
        out += a.position;
        return out;
    }
}
