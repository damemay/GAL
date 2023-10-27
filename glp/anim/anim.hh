#pragma once

#include <vector>
#include <string>

#include "external/glm/glm.hpp"
#include "external/glm/gtc/quaternion.hpp"

#include "transform.hh"
#include "meshes.hh"

namespace anim {

template<typename T>
class Bezier {
    public:
        T p1;
        T c1;
        T p2;
        T c2;
};

template<typename T>
inline T interpolate(Bezier<T>& curve, float t) {
    T a = glm::mix(curve.p1, curve.c1, t);
    T b = glm::mix(curve.c2, curve.p2, t);
    T c = glm::mix(curve.c1, curve.c2, t);

    T d = glm::mix(a, c, t);
    T e = glm::mix(c, b, t);

    return glm::mix(d, e, t);
}

enum class Interpolation {
    constant,
    linear,
    cubic,
};

template<unsigned int N>
class Frame {
    public:
        float value[N];
        float in[N];
        float out[N];
        float time;
};

typedef Frame<1> scalar_frame;
typedef Frame<3> vector_frame;
typedef Frame<4> quat_frame;

namespace track_helpers {
    inline float interp(float a, float b, float t) { return a + (b-a)*t; }
    inline glm::vec3 interp(const glm::vec3& a, const glm::vec3& b, float t) { return glm::mix(a, b, t); }
    inline glm::quat interp(const glm::quat& a, const glm::quat& b, float t) {
        glm::quat result = glm::lerp(a, b, t);
        if(glm::dot(a, b) < 0) result = glm::lerp(a, -b, t);
        return glm::normalize(result);
    }

    inline float adjust_hermite_res(float f) { return f; }
    inline glm::vec3 adjust_hermite_res(const glm::vec3& v) { return v; }
    inline glm::quat adjust_hermite_res(const glm::quat& q) { return glm::normalize(q); }

    inline void neighborhood(const float& a, float& b) {}
    inline void neighborhood(const glm::vec3& a, glm::vec3& b) {}
    inline void neighborhood(const glm::quat& a, glm::quat& b) {
        if(glm::dot(a, b) < 0) {
            b = -b;
        }
    }
}

template<typename T, int N>
class Track {
    protected:
        std::vector<Frame<N>> frames;
        Interpolation interpolation;

        inline T sample_const(float time, bool looping) {
            int frame = frame_idx(time, looping);
            if(frame<0 || frame >= (int)frames.size()) return T();
            return cast(&frames[frame].value[0]);
        }

        inline T sample_lin(float time, bool looping) {
            int frame = frame_idx(time, looping);
            if(frame<0 || frame >= frames.size()-1) return T();
            int n_frame = frame+1;
        
            float track_t = adjust_time_to_fit_track(time, looping);
            float this_t = frames[frame].time;
            float frame_delta = frames[n_frame].time - this_t;
        
            if(frame_delta <= 0.0f) return T();
            float t = (track_t - this_t) / frame_delta;
            
            T start = cast(&frames[frame].value[0]);
            T end = cast(&frames[n_frame].value[0]);
            return track_helpers::interp(start, end, t);
        }

        inline T sample_cub(float time, bool looping) {
            int frame = frame_idx(time, looping);
            if(frame<0 || frame >= frames.size()-1) return T();
            int n_frame = frame+1;
        
            float track_t = adjust_time_to_fit_track(time, looping);
            float this_t = frames[frame].time;
            float frame_delta = frames[n_frame].time - this_t;
        
            if(frame_delta <= 0.0f) return T();
            float t = (track_t - this_t) / frame_delta;
        
            T point1 = cast(&frames[frame].value[0]);
            T slope1;
            memcpy(&slope1, frames[frame].out, N*sizeof(float));
            slope1 *= frame_delta;
        
            T point2 = cast(&frames[n_frame].value[0]);
            T slope2;
            memcpy(&slope2, frames[n_frame].in, N*sizeof(float));
            slope2 *= frame_delta;
            return hermite(t, point1, slope1, point2, slope2);
        }

        inline T hermite(float t, const T& p1, const T& s1, const T& p2, const T& s2) {
            float tt = t*t;
            float ttt = tt*t;
        
            T _p2 = p2;
            track_helpers::neighborhood(p1, _p2);
        
            float h1 = 2.0f * ttt - 3.0f * tt + 1.0f;
            float h2 = -2.0f * ttt + 3.0f * tt;
            float h3 = ttt - 2.0f * tt + t;
            float h4 = ttt - tt;
        
            T result = p1 * h1 + _p2 * h2 + s1 * h3 + s2 * h4;
            return track_helpers::adjust_hermite_res(result);
        }

        inline int frame_idx(float time, bool looping) {
            unsigned int size = (unsigned int)frames.size();
            if(size <= 1) return -1;
            if(looping) {
                float start_t = frames[0].time;
                float end_t = frames[size-1].time;
                float dur = end_t - start_t;
                time = fmodf(time - start_t, end_t - start_t);
                if(time<0.0f) time += end_t-start_t;
                time += start_t;
            } else {
                if(time <= frames[0].time) return 0;
                if(time >= frames[size-2].time) return (int)size-2;
            }
            for(int i=(int)size-1; i>=0; --i)
                if(time>=frames[i].time) return i;
            return -1;
        }

        inline float adjust_time_to_fit_track(float time, bool looping) {
            unsigned int size = (unsigned int)frames.size();
            if(size <= 1) return 0.0f;
            float start_t = frames[0].time;
            float end_t = frames[size-1].time;
            float dur = end_t-start_t;
            if(dur <= 0.0f) return 0.0f;
            if(looping) {
                time = fmodf(time - start_t, end_t - start_t);
                if(time < 0.0f) time += end_t - start_t;
                time += start_t;
            } else {
                if(time <= frames[0].time) time = start_t;
                if(time >= frames[size-1].time) time = end_t;
            }
            return time;
        }

        T cast(float* value);
        
    public:
        inline Track() { interpolation = Interpolation::linear; }

        inline void resize(unsigned int size) { frames.resize(size); }
        inline unsigned int size() { return frames.size(); }
        inline Interpolation get_interpolation() { return interpolation; }
        inline void set_interpolation(Interpolation interp) { interpolation = interp; }
        inline float get_start_time() { return frames[0].time; }
        inline float get_end_time() { return frames[frames.size()-1].time; }

        inline T sample(float time, bool looping) {
            if(interpolation == Interpolation::constant) return sample_const(time, looping);
            else if(interpolation == Interpolation::linear) return sample_lin(time, looping);
            return sample_cub(time, looping);
        }

        inline Frame<N>& operator[](unsigned int index) { return frames[index]; }
};

typedef Track<float, 1> scalar_track;
typedef Track<glm::vec3, 3> vector_track;
typedef Track<glm::quat, 4> quat_track;

class TransformTrack {
    protected:
        unsigned int id {0};
        vector_track position;
        quat_track rotation;
        vector_track scale;
    public:
        TransformTrack() = default;
        inline unsigned int get_id() { return id; }
        inline void set_id(unsigned int id) { id = id; }
        inline vector_track& get_position_track() { return position; }
        inline quat_track& get_rotation_track() { return rotation; }
        inline vector_track& get_scale_track() { return scale; }
        inline bool is_valid() { return position.size() > 1 || rotation.size() > 1 || scale.size() > 1; }
        float get_start_time();
        float get_end_time();
        transform::_t sample(const transform::_t& ref, float time, bool looping);
};

class Clip {
    protected:
        std::vector<TransformTrack> tracks;
        std::string name {"noname"};
        float start_time {0.0f}, end_time {0.0f};
        bool looping {true};

        float adjust_time_to_fit_range(float in_time);

    public:
        Clip() = default;
        float sample(Pose& pose, float in_time);
        TransformTrack& operator[](unsigned int idx);
        void recalc_duration();

        inline unsigned int get_id_at_index(unsigned int idx) { return tracks[idx].get_id(); }
        inline void set_id_at_index(unsigned int idx, unsigned int id) { return tracks[idx].set_id(id); }
        inline unsigned int size() { return (unsigned int)tracks.size(); }
        inline std::string& get_name() { return name; }
        inline void set_name(const std::string& new_name) { name = new_name; }
        inline float get_duration() { return end_time - start_time; }
        inline float get_start_time() { return start_time; }
        inline float get_end_time() { return end_time; }
        inline bool get_looping() { return looping; }
        inline void set_looping(bool loop) { looping = loop; }
};

}
