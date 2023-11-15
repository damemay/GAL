#include "obj/collidable.hh"
#include "obj/renderable.hh"

namespace glp {

namespace Object {

World::World() {
    coll_config = new btDefaultCollisionConfiguration{};
    dispatcher = new btCollisionDispatcher{coll_config};
    broadphase = new btDbvtBroadphase{};
    solver = new btSequentialImpulseConstraintSolver{};
    world = new btDiscreteDynamicsWorld{dispatcher, broadphase, solver, coll_config};
}

World::~World() {
    delete coll_config;
    delete dispatcher;
    delete broadphase;
    delete solver;
}

void CollRenderableModel::update(float dt) {
    btTransform t;
    motion_state->getWorldTransform(t);
    auto rot = t.getRotation();
    auto tr = t.getOrigin();
    transform = glm::translate(glm::mat4(1.0f), glm::vec3(tr.getX(), tr.getY(), tr.getZ()));
    transform = glm::rotate(transform, rot.getAngle(), glm::vec3(rot.getAxis().getX(), rot.getAxis().getY(), rot.getAxis().getZ()));
}

CollRenderableModel::CollRenderableModel(const std::string& path, Shader* shader, ShadingType shading_t, btCollisionShape* shape, float mass, const btVector3& position, const btQuaternion& rotation) : Collidable{shape, mass, position, rotation} {
    load(path, shader, shading_t);
}

CollRenderableModel::CollRenderableModel(const std::string& path, Shader* shader, ShadingType shading_t, float mass, const btVector3& position, const btQuaternion& rotation) : Renderable{path, shader, shading_t} {
    auto box = model->calculate_bounding_box();
    setup(new btBoxShape(btVector3(box.x, box.y, box.z)), mass, position, rotation);
}

CollRenderableModel::CollRenderableModel(Model* model_, Shader* shader_, ShadingType shading_t, btCollisionShape* shape, float mass, const btVector3& position, const btQuaternion& rotation) : Renderable{model_, shader_, shading_t}, Collidable{shape, mass, position, rotation} {}

CollRenderableModel::CollRenderableModel(Model* model, Shader* shader, ShadingType shading_t, float mass, const btVector3& position, const btQuaternion& rotation) : Renderable{model, shader, shading_t} {
    auto box = model->calculate_bounding_box();
    setup(new btBoxShape(btVector3(box.x, box.y, box.z)), mass, position, rotation);
}

CollRenderableModelSeparate::CollRenderableModelSeparate(const std::string& path, Shader* shader, ShadingType shading_t, const btVector3& position, const btQuaternion& rotation) : Renderable{path, shader, shading_t} {
    auto boxes = model->calculate_bounding_boxes();
    for(auto& box: boxes) {
        objects.push_back(new Collidable{new btBoxShape(btVector3(box.x, box.y, box.z)), 0.0f, position, rotation});
    }
    transform = glm::translate(glm::mat4(1.0f), glm::vec3(position.getX(), position.getY(), position.getZ()));
}

CollRenderableModelSeparate::CollRenderableModelSeparate(Model* model, Shader* shader, ShadingType shading_t, const btVector3& position, const btQuaternion& rotation) : Renderable{model, shader, shading_t} {
    auto boxes = model->calculate_bounding_boxes();
    for(auto& box: boxes) {
        objects.push_back(new Collidable{new btBoxShape(btVector3(box.x, box.y, box.z)), 0.0f, position, rotation});
    }
    transform = glm::translate(glm::mat4(1.0f), glm::vec3(position.getX(), position.getY(), position.getZ()));
}

Collidable::Collidable(btCollisionShape* shape_, float mass_, const btVector3& position, const btQuaternion& rotation) : shape{shape_}, mass{mass_} {
    setup(shape_, mass_, position, rotation);
}

void Collidable::setup(btCollisionShape* shape_, float mass_, const btVector3& position, const btQuaternion& rotation) {
    shape = shape_;
    mass = mass_;
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(position);
    transform.setRotation(rotation);

    motion_state = new btDefaultMotionState{transform};

    btVector3 local_inertia {0.0f, 0.0f, 0.0f};
    if(mass != 0.0f) shape->calculateLocalInertia(mass, local_inertia);

    btRigidBody::btRigidBodyConstructionInfo info {mass, motion_state, shape, local_inertia};
    rigidbody = new btRigidBody{info};
}

Collidable::~Collidable() {
    delete rigidbody;
    delete shape;
    delete motion_state;
}

}

}
