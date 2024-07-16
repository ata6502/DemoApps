#include "pch.h"

#include "Constants.h"
#include "Swarm.h"

using namespace Concurrency;
using namespace DirectX;

Swarm::Swarm(
    float boidRadius, 
    float boidMinDistance, 
    float boidMatchingFactor, 
    float maxBoidSpeed,
    float boidAvoidFactor,
    float boidTurnFactor,
    float boidVisualRange,
    float boidMoveToCenterFactor) :
    m_boidRadius(boidRadius)
{
    m_boidParameters[BoidParameter::MinDistance] = m_boidRadius + boidMinDistance;
    m_boidParameters[BoidParameter::MatchingFactor] = boidMatchingFactor;
    m_boidParameters[BoidParameter::MaxSpeed] = maxBoidSpeed;
    m_boidParameters[BoidParameter::AvoidFactor] = boidAvoidFactor;
    m_boidParameters[BoidParameter::TurnFactor] = boidTurnFactor;
    m_boidParameters[BoidParameter::VisualRange] = boidVisualRange;
    m_boidParameters[BoidParameter::MoveToCenterFactor] = boidMoveToCenterFactor;

    m_rand = std::make_unique<RandomNumberHelper>();
}

void Swarm::AddBoids(int count)
{
    critical_section::scoped_lock lock(m_criticalSection);

    for (auto i = 0; i < count; ++i)
    {
        auto [randomPosition, randomVelocity] = GetRandomPositionAndVelocity();
        m_boids.emplace_back(std::make_unique<Boid>(randomPosition, randomVelocity, m_boidParameters[BoidParameter::MaxSpeed]));
    }
}

void Swarm::RemoveBoids(int count)
{
    critical_section::scoped_lock lock(m_criticalSection);

    if (count >= Size())
        m_boids.clear();
    else
    {
        auto end = std::end(m_boids);
        m_boids.erase(end - count, end);
    }
}

void Swarm::Update(float timeDelta)
{
    critical_section::scoped_lock lock(m_criticalSection);

    XMVECTOR v1, v2, v3, v4;

    for (int i = 0; i < Size(); ++i)
    {
        // Perform vector operations on the positions of the boids. Operations are independent from each other.

        // Rule 1: Make boids fly towards the centre of the mass of neighbouring boids.
        v1 = ExecuteRule1(i);

        // Rule 2: Move away from other boids that are too close to avoid colliding.
        v2 = ExecuteRule2(i);

        // Rule 3: Find the average velocity (speed and direction) of the other boids and adjust velocity slightly to match.
        v3 = ExecuteRule3(i, true);

        // Rule 4: Encourage boids to stay within rough boundaries.
        v4 = ExecuteRule4(i);

        XMVECTOR velocityDelta = timeDelta * (v1 + v2 + v3 + v4);

        m_boids[i]->Update(velocityDelta);
    }
}

void Swarm::ResetBoids()
{
    critical_section::scoped_lock lock(m_criticalSection);

    for (auto i = 0; i < Size(); ++i)
    {
        auto [randomPosition, randomVelocity] = GetRandomPositionAndVelocity();
        m_boids[i]->SetPosition(randomPosition);
        m_boids[i]->SetVelocity(randomVelocity);
    }
}

void Swarm::Iterate(std::function<void(DirectX::XMMATRIX)> function)
{
    critical_section::scoped_lock lock(m_criticalSection);

    for (auto i = 0; i < Size(); ++i)
    {
        function(m_boids[i]->GetWorldMatrix());
    }
}

float Swarm::GetBoidParameter(BoidParameter parameter)
{
    return m_boidParameters[parameter];
}

void Swarm::SetBoidParameter(BoidParameter parameter, float value)
{
    switch (parameter)
    {
    case BoidParameter::MinDistance:
        m_boidParameters[BoidParameter::MinDistance] = m_boidRadius + value;
        break;
    case BoidParameter::MaxSpeed:
        SetMaxBoidSpeed(value);
        break;
    default:
        m_boidParameters[parameter] = value;
        break;
    }
}

// Move the boid toward its 'perceived centre', which is the centre of all the other boids, not including itself.
DirectX::XMVECTOR Swarm::ExecuteRule1(int boidIndex)
{
    // Add all boids' positions except the boid with the boidIndex.
    XMVECTOR sum{ XMVectorZero() };
    for (int i = 0; i < Size(); ++i)
    {
        if (boidIndex != i)
            sum = XMVectorAdd(sum, m_boids[i]->GetPosition());
    }

    size_t boidCount = Size() - 1; // all the boids minus the current boid
    XMVECTOR centre = sum / (static_cast<float>(boidCount)); // the center of mass
    XMVECTOR boidPosition = m_boids[boidIndex]->GetPosition();
    XMVECTOR v = XMVectorSubtract(centre, boidPosition) * m_boidParameters[BoidParameter::MoveToCenterFactor];

    return v;
}

// Move away from other boids that are too close to avoid colliding.
DirectX::XMVECTOR Swarm::ExecuteRule2(int boidIndex)
{
    XMVECTOR moveDelta{ XMVectorZero() };

    for (int i = 0; i < Size(); ++i)
    {
        if (boidIndex != i)
        {
            // Calculate the distance of this boid to the other boid.
            auto p1 = m_boids[boidIndex]->GetPosition();
            auto p2 = m_boids[i]->GetPosition();
            auto diff = XMVectorSubtract(p2, p1);
            auto distance = XMVectorGetX(XMVector3Length(diff));

            // Accumulate the displacement of each boid that is nearby.
            if (distance < m_boidParameters[BoidParameter::MinDistance])
                moveDelta = XMVectorSubtract(moveDelta, diff);
        }
    }

    XMVECTOR v = m_boidParameters[BoidParameter::AvoidFactor] * moveDelta;
    return v;
}

// Adjust the boid's velocity to match the average velocity of the other boids.
DirectX::XMVECTOR Swarm::ExecuteRule3(int boidIndex, bool allBoids)
{
    float matchingFactor = m_boidParameters[BoidParameter::MatchingFactor];

    // Method #1: Take into account all boids.
    if (allBoids)
    {
        XMVECTOR sum{ XMVectorZero() };

        for (int i = 0; i < Size(); ++i)
        {
            if (boidIndex != i)
                sum = XMVectorAdd(sum, m_boids[i]->GetVelocity());
        }

        size_t boidCount = Size() - 1; // all the boids minus the current boid
        XMVECTOR centre = sum / static_cast<float>(boidCount);
        XMVECTOR boidVelocity = m_boids[boidIndex]->GetVelocity();
        XMVECTOR v = XMVectorSubtract(centre, boidVelocity) * matchingFactor;

        return v;
    }
    // Method #2: Take into account only boids in a certain range from a given boid.
    else
    {
        XMVECTOR avg{ XMVectorZero() };
        int neighborCount = 0;
        float visualRange = m_boidParameters[BoidParameter::VisualRange];

        for (int i = 0; i < Size(); ++i)
        {
            if (boidIndex != i)
            {
                // Calculate the distance of this boid to the other boid.
                auto p1 = m_boids[boidIndex]->GetPosition();
                auto p2 = m_boids[i]->GetPosition();
                auto diff = XMVectorSubtract(p2, p1);
                auto distance = XMVectorGetX(XMVector3Length(diff));

                if (distance < visualRange)
                {
                    avg = XMVectorAdd(avg, m_boids[i]->GetVelocity());
                    ++neighborCount;
                }
            }
        }

        XMVECTOR v = XMVectorZero();

        if (neighborCount > 0)
        {
            XMVECTOR centre = avg / static_cast<float>(neighborCount);
            XMVECTOR boidVelocity = m_boids[boidIndex]->GetVelocity();
            v = XMVectorSubtract(centre, boidVelocity) * matchingFactor;
        }

        return v;
    }
}

// Keeps the boid within bounds. The boids can fly out of boundaries, but then slowly turn back, avoiding any harsh motions
DirectX::XMVECTOR Swarm::ExecuteRule4(int boidIndex)
{
    XMFLOAT3 v;
    ZeroMemory(&v, sizeof(v));
    XMFLOAT3 pos;
    ZeroMemory(&pos, sizeof(pos));
    XMStoreFloat3(&pos, m_boids[boidIndex]->GetPosition());

    float turnFactor = m_boidParameters[BoidParameter::TurnFactor];

    if (pos.x < BOUNDARY_X_MIN)
        v.x = turnFactor;
    else if (pos.x > BOUNDARY_X_MAX)
        v.x = -turnFactor;

    if (pos.y < BOUNDARY_Y_MIN)
        v.y = turnFactor;
    else if (pos.y > BOUNDARY_Y_MAX)
        v.y = -turnFactor;

    if (pos.z < BOUNDARY_Z_MIN)
        v.z = turnFactor;
    else if (pos.z > BOUNDARY_Z_MAX)
        v.z = -turnFactor;

    return XMLoadFloat3(&v);
}

std::tuple<DirectX::XMVECTOR, DirectX::XMVECTOR> Swarm::GetRandomPositionAndVelocity()
{
    XMVECTOR randomPosition = XMVectorSet(
        m_rand->GetFloat(8 * BOUNDARY_X_MIN, 8 * BOUNDARY_X_MAX),
        m_rand->GetFloat(2.1f * BOUNDARY_Y_MAX, 7 * BOUNDARY_Y_MAX),
        m_rand->GetFloat(2.5f * BOUNDARY_Z_MIN, 10 * BOUNDARY_Z_MIN),
        0);

    XMVECTOR randomVelocity = m_boidParameters[BoidParameter::MaxSpeed] *
        XMVector4Normalize(
            XMVectorSet(
                m_rand->GetFloat(-1, 1),
                m_rand->GetFloat(-1, 1),
                m_rand->GetFloat(-1, 1),
                0));

    return { randomPosition, randomVelocity };
}

void Swarm::SetMaxBoidSpeed(float maxBoidSpeed)
{
    m_boidParameters[BoidParameter::MaxSpeed] = maxBoidSpeed;

    // Update max speed of all boids in the swarm.
    for (auto i = 0; i < Size(); ++i)
    {
        m_boids[i]->SetMaxSpeed(maxBoidSpeed);
    }
}
    