#pragma once

enum class BoidParameter
{
    MinDistance,        // the minimum distance between boids
    MaxSpeed,           // the max length of the velocity vector
    MoveToCenterFactor, // determines how to move a boid towards the center (percentage)
    MatchingFactor,     // adjustment of average velocity as % (boid matching factor)
};
