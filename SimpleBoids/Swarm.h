#pragma once

#include "Boid.h"
#include "BoidParameter.h"
#include "RandomNumberHelper.h"

#include <functional>
#include <map>
#include <tuple>

class Swarm
{
public:
    Swarm(
        float boidRadius, 
        float boidMinDistance, 
        float boidMatchingFactor, 
        float maxBoidSpeed,
        float boidMoveToCenterFactor);

    // Creates or destroys the given number of boids.
    void AddBoids(int count);
    void RemoveBoids(int count);

    // Moves all the boids to new positions.
    void Update(float timeDelta);

    // Moves boids to initial random positions.
    void ResetBoids();

    // Iterates over all boids executing a given function.
    void Iterate(std::function<void(DirectX::XMMATRIX)> function);

    // Accessors
    size_t Size() const { return m_boids.size(); }
    float GetBoidParameter(BoidParameter parameter);
    void SetBoidParameter(BoidParameter parameter, float value);

private:
    Concurrency::critical_section               m_criticalSection;
    std::vector<std::unique_ptr<Boid>>          m_boids;
    std::unique_ptr<RandomNumberHelper>         m_rand;
    float                                       m_boidRadius;
    std::map<BoidParameter, float>              m_boidParameters;

    DirectX::XMVECTOR ExecuteRule1(int boidIndex);
    DirectX::XMVECTOR ExecuteRule2(int boidIndex);
    DirectX::XMVECTOR ExecuteRule3(int boidIndex, bool allBoids = true);
    DirectX::XMVECTOR ExecuteRule4(int boidIndex);

    std::tuple<DirectX::XMVECTOR, DirectX::XMVECTOR> GetRandomPositionAndVelocity();
    void SetMaxBoidSpeed(float maxBoidSpeed);
};

