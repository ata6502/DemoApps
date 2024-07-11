#pragma once

const float BOUNDARY_X_MIN = -40.0f;
const float BOUNDARY_X_MAX = 40.0f;
const float BOUNDARY_Y_MIN = -40.0f;
const float BOUNDARY_Y_MAX = 40.0f;
const float BOUNDARY_Z_MIN = -40.0f;
const float BOUNDARY_Z_MAX = 40.0f;
const float BOUNDARY_THICKNESS = 2.f;

const int INITIAL_BOID_COUNT = 210;

// Boid parameters
const float BOID_AVOID_FACTOR = 0.3f;               // scales the vectors that repel boids
const float BOID_TURN_FACTOR = 0.5f;                // encourages boids to fly in a particular direction
const float BOID_VISUAL_RANGE = 3.0f;               // used in calculating boid's velocity while taking into account only boids in a certain range
