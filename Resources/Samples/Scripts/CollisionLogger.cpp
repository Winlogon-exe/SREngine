//
// Created by innerviewer on 21.01.2023.
//

#include <Libraries/Utils/Allocator.h>
#include <Libraries/Types/Behaviour.h>

#include <Libraries/Debug.h>
#include <Libraries/Input.h>

class CollisionLogger : public Behaviour {
public:
    void OnCollisionEnter(const CollisionData& data) override {
        Debug::Log("Collision ENTER detected!");

        /// Dividing impulse value by the simulation time step allows to get an impulse force value.
        std::cout << (data.impulse.x/fixedDeltaTime) << std::endl;
        std::cout << (data.impulse.y/fixedDeltaTime) << std::endl;
        std::cout << (data.impulse.z/fixedDeltaTime) << std::endl;
    }
    void OnCollisionStay(const CollisionData& data) override {
        Debug::Log("Collision STAY detected!");
    }
    void OnCollisionExit(const CollisionData& data) override {
        Debug::Log("Collision EXIT detected!");
    }
    void OnTriggerEnter(const CollisionData& data) override {
        Debug::Log("Trigger ENTER detected!");
    }
    void OnTriggerExit(const CollisionData& data) override {
        Debug::Log("Trigger EXIT detected!");
    }
};

REGISTER_BEHAVIOUR(CollisionLogger)
