#ifndef ACTOR_H
#define ACTOR_H

// structures

typedef struct
{

	float idle_acceleration_rate;
	float walk_acceleration_rate;
	float run_acceleration_rate;
	float roll_acceleration_rate;
	float roll_acceleration_grip_rate;
	float jump_acceleration_rate;
	float aerial_control_rate;

	float walk_target_speed;
	float run_target_speed;
	float sprint_target_speed;
	float idle_to_roll_target_speed;
	float idle_to_roll_grip_target_speed;
	float walk_to_roll_target_speed;
	float run_to_roll_target_speed;
	float sprint_to_roll_target_speed;
	float jump_target_speed;

	float jump_timer_max;

	float fall_max_speed;
	float jump_max_speed;
	float jump_horizontal_boost;

} ActorSettings;

typedef struct
{

	float stick_magnitude;
	float stick_x;
	float stick_y;
	float jump_time_held;
	float jump_time_buffer;
	bool jump_hold;
	bool jump_released;

} Actorinput;

typedef struct
{

	T3DSkeleton main;
	T3DSkeleton blend;

} ActorArmature;

typedef struct
{

	T3DAnim breathing_idle;
	T3DAnim running_left;
	T3DAnim jump_left;
	T3DAnim falling_left;
	T3DAnim land_left;

} AnimationSet;

typedef struct
{

	uint8_t previous;
	uint8_t current;

	AnimationSet main;
	AnimationSet blend;

	uint8_t change_delay;
	float blending_ratio;
	float speed_rate;
	bool synced;

} ActorAnimation;

typedef struct
{

	uint32_t id;
	rspq_block_t *dl;
	T3DMat4FP *modelMat;
	T3DModel *model;
	Vector3 scale;

	char model_path;
	ActorArmature armature;
	ActorAnimation animation;

	RigidBody body;

	float target_yaw;
	float horizontal_target_speed;
	Vector3 target_velocity;

	float horizontal_speed;
	bool grounded;
	float grounding_height;

	bool hasCollided; // Testing a collision boolean

	uint8_t locomotion_state;
	uint8_t previous_state;
	uint8_t state;

	Vector3 home;

	ActorSettings settings;
	Actorinput input;

} Actor;

// function prototypes

Actor actor_create(uint32_t id, const char *model_path);

void actor_draw(Actor *actor);
void actor_delete(Actor *actor);

// function implemenations

Actor actor_create(uint32_t id, const char *model_path)
{
	Actor actor = {

		.id = id,
		.model = t3d_model_load(model_path),
		.modelMat = malloc_uncached(sizeof(T3DMat4FP)),

		.scale = {1.0f, 1.0f, 1.0f},

		.state = 1,
		.previous_state = 1,
		.locomotion_state = 1,

		.body = {
			.position = {0.0f, 0.0f, 0.0f},
			.velocity = {0.0f, 0.0f, 0.0f},
			.rotation = {0.0f, 0.0f, 0.0f},
		},

		.grounding_height = -2000.0f, // magic number

		.settings = {.idle_acceleration_rate = 9, .walk_acceleration_rate = 4, .run_acceleration_rate = 10, .roll_acceleration_rate = 20, .roll_acceleration_grip_rate = 2, .jump_acceleration_rate = 50, .aerial_control_rate = 4.0, .walk_target_speed = 200, .run_target_speed = 700, .sprint_target_speed = 900, .idle_to_roll_target_speed = 300, .idle_to_roll_grip_target_speed = 50, .walk_to_roll_target_speed = 400, .run_to_roll_target_speed = 780, .sprint_to_roll_target_speed = 980, .jump_target_speed = 500, .jump_timer_max = 0.20, .fall_max_speed = -2650.0f, .jump_max_speed = 1000.0f, .jump_horizontal_boost = 100.0f},
	};

	actor.armature.main = t3d_skeleton_create(actor.model);
	// actor.armature.blend = t3d_skeleton_clone(&actor.armature.main, false);

	rspq_block_begin();
	t3d_matrix_set(actor.modelMat, true);
	t3d_model_draw_skinned(actor.model, &actor.armature.main);
	actor.dl = rspq_block_end();

	t3d_mat4fp_identity(actor.modelMat);

	return actor;
}

void actor_updateMat(Actor *actor)
{
	if (actor->state == 9)
		return; // DEATH

	t3d_mat4fp_from_srt_euler(actor->modelMat,
							  (float[3]){actor->scale.x, actor->scale.y, actor->scale.z},
							  (float[3]){rad(actor->body.rotation.x), rad(actor->body.rotation.y), rad(actor->body.rotation.z)},
							  (float[3]){actor->body.position.x, actor->body.position.y, actor->body.position.z});
}

void actor_draw(Actor *actor)
{
	for (uint8_t i = 0; i < ACTOR_COUNT; i++)
	{

		if (actor[i].state == 9)
			continue; // DEATH

		rspq_block_run(actor[i].dl);
	};
}

void actor_delete(Actor *actor)
{
	free_uncached(actor->modelMat);

	t3d_skeleton_destroy(&actor->armature.main);
	t3d_anim_destroy(&actor->animation.main.breathing_idle);
	t3d_anim_destroy(&actor->animation.main.running_left);
	t3d_anim_destroy(&actor->animation.main.falling_left);
	// t3d_anim_destroy(&actor->animation.main.jump_left);
	// t3d_anim_destroy(&actor->animation.main.land_left);

	// t3d_skeleton_destroy(&actor->armature.blend);
	// t3d_anim_destroy(&actor->animation.blend.breathing_idle);
	// t3d_anim_destroy(&actor->animation.blend.running_left);
	// t3d_anim_destroy(&actor->animation.blend.falling_left);
	// t3d_anim_destroy(&actor->animation.blend.jump_left);
	// t3d_anim_destroy(&actor->animation.blend.land_left);

	t3d_model_free(actor->model);
	if (actor->dl != NULL)
		rspq_block_free(actor->dl);
}

#endif