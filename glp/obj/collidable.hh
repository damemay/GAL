#pragma once

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "obj/renderable.hh"

namespace glp {

namespace Object {

class CollidableInterface {
    public:
        virtual ~CollidableInterface() {};
};

class Collidable : public CollidableInterface {
    protected:
        float mass{1.0f};
        btRigidBody* rigidbody {nullptr};
        btCollisionShape* shape {nullptr};
        btDefaultMotionState* motion_state {nullptr};

    public:
        inline btRigidBody* get_rigidbody() { return rigidbody; }

        void setup(btCollisionShape* shape, float mass, const btVector3& position=btVector3{0.0f, 0.0f, 0.0f}, const btQuaternion& rotation=btQuaternion{0, 0, 0, 1});

        Collidable(btCollisionShape* shape, float mass, const btVector3& position=btVector3{0.0f, 0.0f, 0.0f}, const btQuaternion& rotation=btQuaternion{0, 0, 0, 1});
        Collidable() {};

        virtual ~Collidable();
        Collidable(const Collidable&) = delete;
        Collidable& operator=(const Collidable&) = delete;

};

class CollRenderableModel : public Collidable, public Renderable {
    public:
        void update(float dt);

        CollRenderableModel(const std::string& path, Shader* shader, ShadingType shading_t, btCollisionShape* shape, float mass, const btVector3& position=btVector3{0.0f, 0.0f, 0.0f}, const btQuaternion& rotation=btQuaternion{0, 0, 0, 1});
        CollRenderableModel(const std::string& path, Shader* shader, ShadingType shading_t, float mass, const btVector3& position=btVector3{0.0f, 0.0f, 0.0f}, const btQuaternion& rotation=btQuaternion{0, 0, 0, 1});

        CollRenderableModel(Model* model, Shader* shader, ShadingType shading_t, btCollisionShape* shape, float mass, const btVector3& position=btVector3{0.0f, 0.0f, 0.0f}, const btQuaternion& rotation=btQuaternion{0, 0, 0, 1});
        CollRenderableModel(Model* model, Shader* shader, ShadingType shading_t, float mass, const btVector3& position=btVector3{0.0f, 0.0f, 0.0f}, const btQuaternion& rotation=btQuaternion{0, 0, 0, 1});

};

class CollRenderableModelSeparate : public CollidableInterface, public Renderable {
    public:
        std::vector<Collidable*> objects;

        CollRenderableModelSeparate(const std::string& path, Shader* shader, ShadingType shading_t, const btVector3& position=btVector3{0.0f, 0.0f, 0.0f}, const btQuaternion& rotation=btQuaternion{0, 0, 0, 1});
        CollRenderableModelSeparate(Model* model, Shader* shader, ShadingType shading_t, const btVector3& position=btVector3{0.0f, 0.0f, 0.0f}, const btQuaternion& rotation=btQuaternion{0, 0, 0, 1});
};

class World {
    protected:
        btCollisionConfiguration* coll_config;
        btCollisionDispatcher* dispatcher;
        btBroadphaseInterface* broadphase;
        btConstraintSolver* solver;
        btDynamicsWorld* world;

        std::vector<CollidableInterface*> objects;

    public:
        inline void add_collidable(CollidableInterface* c) {
            auto collidable = dynamic_cast<Collidable*>(c);
            auto separate = dynamic_cast<CollRenderableModelSeparate*>(c);
            if(collidable) {
                world->addRigidBody(collidable->get_rigidbody());
                objects.push_back(c);
            } else if(separate) {
                for(auto& obj: separate->objects) {
                    world->addRigidBody(obj->get_rigidbody());
                    objects.push_back(obj);
                }
                objects.push_back(separate);
            }
        }

        inline void render(Camera& camera) {
            for(auto& obj: objects) {
                auto renderable = dynamic_cast<Renderable*>(obj);
                if(renderable) renderable->render(camera);
            }
        }

        inline void update(float dt) {
            for(auto& obj: objects) {
                auto renderable = dynamic_cast<CollRenderableModel*>(obj);
                if(renderable) renderable->update(dt);
            }
            world->stepSimulation(dt);
        }

        World();
        ~World();
};

}

}
