#pragma once

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "obj/renderable.hh"

namespace glp {

namespace Object {
    
class BulletDebugDraw : public btIDebugDraw {
    protected:
        GLuint VAO, VBO;
        std::string fragment = "#version 330 core\n"
            "in vec3 out_color;\n"
            "out vec4 color;\n"
            "void main() { color = vec4(out_color, 1.0);   }\n";
        std::string vertex = "#version 330 core\n"
            "layout (location = 0) in vec3 position;\n"
            "layout (location = 1) in vec3 color;\n"
            "out vec3 out_color;\n"
            "uniform mat4 vp;\n"
            "void main() {\n"
            "gl_Position = vp * vec4(position, 1.0f);\n"
            "out_color = color;}\n";
        Shader shader {vertex, fragment, false};

    public:
        BulletDebugDraw() {
            glGenBuffers(1, &VBO);
            glGenVertexArrays(1, &VAO);
        }

        ~BulletDebugDraw() {
            glDeleteBuffers(1, &VBO);
            glDeleteVertexArrays(1, &VAO);
        }

        inline Shader* get_shader() { return &shader; }

        inline void set_matrices(Camera& cam) {
            shader.bind();
            shader.set("vp", cam.view_projection());
	}

	int m;

        inline virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override {
            if(!m) return;
	    // Vertex data
	    GLfloat points[12];

	    points[0] = from.x();
	    points[1] = from.y();
	    points[2] = from.z();
	    points[3] = color.x();
	    points[4] = color.y();
	    points[5] = color.z();

	    points[6] = to.x();
	    points[7] = to.y();
	    points[8] = to.z();
	    points[9] = color.x();
	    points[10] = color.y();
	    points[11] = color.z();

	    glBindVertexArray(VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, VBO);
	    glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
	    glEnableVertexAttribArray(0);
	    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
	    glEnableVertexAttribArray(1);
	    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	    glBindVertexArray(0);

	    glBindVertexArray(VAO);
	    glDrawArrays(GL_LINES, 0, 2);
	    glBindVertexArray(0);
	}

        virtual void drawContactPoint(const btVector3 &, const btVector3 &, btScalar, int, const btVector3 &) override {}
	virtual void reportErrorWarning(const char *) override {}
	virtual void draw3dText(const btVector3 &, const char *) override {}
	virtual void setDebugMode(int p) override { m = p; }
	int getDebugMode(void) const override { return 3; }
};

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

class World {
    protected:
        btCollisionConfiguration* coll_config;
        btCollisionDispatcher* dispatcher;
        btBroadphaseInterface* broadphase;
        btConstraintSolver* solver;
        btDynamicsWorld* world;

        std::vector<CollidableInterface*> objects;

    public:
        inline btDynamicsWorld* get_bullet_world() { return world; }

        inline void add_collidable(CollidableInterface* c) {
            auto collidable = dynamic_cast<Collidable*>(c);
            if(collidable) {
                world->addRigidBody(collidable->get_rigidbody());
                objects.push_back(c);
            }        }

        inline void render(Camera& camera) {
            for(auto& obj: objects) {
                auto renderable = dynamic_cast<Renderable*>(obj);
                if(renderable) renderable->render(camera);
            }
            auto deb = dynamic_cast<BulletDebugDraw*>(world->getDebugDrawer());
            if(deb) {
                deb->set_matrices(camera);
                world->debugDrawWorld();
                deb->get_shader()->unbind();
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
