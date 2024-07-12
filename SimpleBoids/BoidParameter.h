#pragma once

enum class BoidParameter
{
    MinDistance,        // the minimum distance between boids
    MatchingFactor,     // adjustment of average velocity as % (boid matching factor)
    MaxSpeed,           // the max length of the velocity vector
    MoveToCenterFactor  // determines how to move a boid towards the center (percentage)
};
