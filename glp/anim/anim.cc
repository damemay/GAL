#include "anim.hh"
#include <cmath>
#include <cstring>

namespace anim {

template<>
float Track<float, 1>::cast(float* value) { return value[0]; }

template<>
glm::vec3 Track<glm::vec3, 3>::cast(float* value) { return glm::vec3(value[0], value[1], value[3]); }

template<>
glm::quat Track<glm::quat, 4>::cast(float* value) {
    return glm::normalize(glm::quat(value[0], value[1], value[2], value[3]));
}

float TransformTrack::get_start_time() {
    float result = 0.0f;
    bool set = false;

    if(position.size() > 1) {
        result = position.get_start_time();
        set = true;
    }
    if(rotation.size() > 1) {
        float rot_start = rotation.get_start_time();
        if(rot_start < result || !set) {
            result = rot_start;
            set = true;
        }
    }
    if(scale.size() > 1) {
        float scale_start = scale.get_start_time();
        if(scale_start < result || !set) {
            result = scale_start;
            set = true;
        }
    }
    return result;
}

float TransformTrack::get_end_time() {
    float result = 0.0f;
    bool set = false;

    if(position.size() > 1) {
        result = position.get_end_time();
        set = true;
    }
    if(rotation.size() > 1) {
        float rot_start = rotation.get_end_time();
        if(rot_start > result || !set) {
            result = rot_start;
            set = true;
        }
    }
    if(scale.size() > 1) {
        float scale_start = scale.get_end_time();
        if(scale_start > result || !set) {
            result = scale_start;
            set = true;
        }
    }
    return result;
}

transform::_t TransformTrack::sample(const transform::_t& ref, float time, bool looping) {
    transform::_t res = ref;
    if(position.size() > 1) res.position = position.sample(time, looping);
    if(rotation.size() > 1) res.rotation = rotation.sample(time, looping);
    if(scale.size() > 1) res.scale = scale.sample(time, looping);
    return res;
}

float Clip::adjust_time_to_fit_range(float in_time) {
    if(looping) {
        float dur = end_time = start_time;
        if(dur<=0) return 0.0f;
        in_time = fmodf(in_time - start_time, end_time - start_time);
        if(in_time<0.0f) in_time += end_time - start_time;
        in_time += start_time;
    } else {
        if(in_time < start_time) in_time = start_time;
        if(in_time > end_time) in_time = end_time;
    }
    return in_time;
}

float Clip::sample(Pose& pose, float time) {
    if(!get_duration()) return 0.0f;
    time = adjust_time_to_fit_range(time);
    unsigned int size = tracks.size();
    for(unsigned int i=0; i<size; ++i) {
        unsigned int j = tracks[i].get_id();
        transform::_t local = pose.get_loc_transform(j);
        transform::_t animated = tracks[i].sample(local, time, looping);
        pose.set_loc_transform(j, animated);
    }
    return time;
}

void Clip::recalc_duration() {
    start_time = 0.0f;
    end_time = 0.0f;
    bool start_s = false;
    bool end_s = false;
    unsigned int tracks_size = tracks.size();
    for(unsigned int i=0; i<tracks_size; ++i) {
        if(tracks[i].is_valid()) {
            float start_t = tracks[i].get_start_time();
            float end_t = tracks[i].get_end_time();
            if(start_t < start_time || !start_s) {
                start_time = start_t;
                start_s = true;
            }
            if(end_t > end_time || !end_s) {
                end_time = end_t;
                end_s = true;
            }
        }
    }
}

TransformTrack& Clip::operator[](unsigned int joint) {
    for(int i=0, s=tracks.size(); i<s; ++i)
        if(tracks[i].get_id() == joint) return tracks[i];
    tracks.push_back(TransformTrack());
    tracks[tracks.size()-1].set_id(joint);
    return tracks[tracks.size()-1];
}

}
