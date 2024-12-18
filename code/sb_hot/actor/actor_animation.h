#ifndef ACTOR_ANIMATION_H
#define ACTOR_ANIMATION_H

// function implemenations

ActorAnimation actorAnimation_create(const Actor *actor)
{
	ActorAnimation animation;
	animation.previous = STAND_IDLE;
	animation.current = STAND_IDLE;
	animation.blending_ratio = 0.0f;
	return animation;
}

void animationSet_init(const Actor *actor, AnimationSet *set)
{
	set->breathing_idle = t3d_anim_create(actor->model, "breathing-idle");
	set->running_left = t3d_anim_create(actor->model, "running-10-left");
	// set->jump_left = t3d_anim_create(actor->model, "jump-left");
	set->falling_left = t3d_anim_create(actor->model, "falling-idle-left");
	// set->land_left = t3d_anim_create(actor->model, "falling-to-landing-left");
}

void actorAnimation_init(const Actor *actor, ActorAnimation *animation)
{
	animationSet_init(actor, &animation->main);
	// animationSet_init(actor, &animation->blend);
	//  attach main
	t3d_anim_attach(&animation->main.breathing_idle, &actor->armature.main);
	t3d_anim_attach(&animation->main.falling_left, &actor->armature.main);
	t3d_anim_attach(&animation->main.running_left, &actor->armature.main);

	// attach blend
	// t3d_anim_attach(&animation->blend.running_left, &actor->armature.blend);
}

void actorAnimation_setStandIdle(Actor *actor, ActorAnimation *animation, const float frame_time, rspq_syncpoint_t *syncpoint)
{
	t3d_anim_update(&animation->main.breathing_idle, frame_time);
}

void actorAnimation_setRunning(Actor *actor, ActorAnimation *animation, const float frame_time, rspq_syncpoint_t *syncpoint)
{
	// if (animation->previous == STAND_IDLE || animation->current == STAND_IDLE) {
	//	animation->blending_ratio = actor->horizontal_speed / 320;
	//	if (animation->blending_ratio > 1.0f) animation->blending_ratio = 1.0f;
	//	if (animation->current == STAND_IDLE) t3d_anim_set_time(&animation->blend.running_left, 0.0f);
	//	if(animation->blending_ratio > 0.0f && animation->blending_ratio < 1.0f)
	//	{
	//		t3d_anim_update(&animation->main.breathing_idle, frame_time);
	//		t3d_anim_set_speed(&animation->blend.running_left, animation->blending_ratio);
	//		t3d_anim_update(&animation->blend.running_left, frame_time);
	//
	//		t3d_skeleton_blend(&actor->armature.main, &actor->armature.main, &actor->armature.blend, animation->blending_ratio);
	//	}
	// }
	// else
	t3d_anim_update(&animation->main.running_left, frame_time);
}

void actorAnimation_setJump(Actor *actor, ActorAnimation *animation, const float frame_time, rspq_syncpoint_t *syncpoint)
{
	t3d_anim_update(&animation->main.falling_left, frame_time);
}

void actor_setAnimation(Actor *actor, ActorAnimation *animation, const float frame_time, rspq_syncpoint_t *syncpoint)
{
	switch (actor->state)
	{

	case STAND_IDLE:
	{
		actorAnimation_setStandIdle(actor, animation, frame_time, syncpoint);
		if (animation->current != STAND_IDLE)
		{
			animation->previous = animation->current;
			animation->current = STAND_IDLE;
		}
		break;
	}

	case RUNNING:
	{
		actorAnimation_setRunning(actor, animation, frame_time, syncpoint);
		if (animation->current != RUNNING)
		{
			animation->previous = animation->current;
			animation->current = RUNNING;
		}
		break;
	}

	case JUMP:
	{
		actorAnimation_setJump(actor, animation, frame_time, syncpoint);
		break;
	}
	case FALLING:
	{
		actorAnimation_setJump(actor, animation, frame_time, syncpoint);
		break;
	}
	case DEATH:
	{
		actorAnimation_setJump(actor, animation, frame_time, syncpoint);
		break;
	}
	}

	/*
	 * The original engine only accounts for 1 actor,
	 * but looping through the actor update for each
	 * means that the syncpoint would be updated
	 * 4 times, which is 3 too many.
	 */
	// if(syncpoint)rspq_syncpoint_wait(*syncpoint);
	// t3d_skeleton_update(&actor->armature.main);
}

// temporary place for this until i solve the circular dependency
void actor_init(Actor *actor)
{
	actor->animation = actorAnimation_create(actor);
	actorAnimation_init(actor, &actor->animation);
}

void actor_update(Actor *actor, ControllerData *control, TimeData *timing, float camera_angle_around, float camera_offset, rspq_syncpoint_t *syncpoint)
{
	if (control != NULL)
		actor_setControlData(actor, control, timing->frame_time_s, camera_angle_around, camera_offset);
	if (actor->previous_state != actor->state)
		actor_setState(actor, actor->state); // Skip setting state if it hasn't changed
	actor_setAnimation(actor, &actor->animation, timing->frame_time_s, syncpoint);
	actor_setMotion(actor, timing->fixed_time_s);
}

#endif