#ifndef CAMERA_H
#define CAMERA_H

#include "light.h"

// structures

typedef enum
{
	ORBITAL,
	AIMING,
	MINIGAME,

} CameraState;

typedef struct
{

	float orbitational_acceleration_rate;
	Vector2 orbitational_max_velocity;

	float zoom_acceleration_rate;
	float zoom_deceleration_rate;
	float zoom_max_speed;

	float distance_from_baricenter;

	float field_of_view;
	float field_of_view_aim;

	float offset_acceleration_rate;
	float offset_deceleration_rate;
	float offset_max_speed;

	float offset_angle;
	float offset_angle_aim;

	float max_pitch;

} CameraSettings;

typedef struct
{

	T3DViewport viewport;

	Vector3 position;
	float offset_height;

	float distance_from_barycenter; // the barycenter is choosen and it's the center of the orbitational movement
	float angle_around_barycenter;
	float pitch;

	float horizontal_barycenter_distance;
	float vertical_barycenter_distance;

	Vector3 target; // target as in the place at which the camera must aim
	float target_distance;
	float horizontal_target_distance;
	float vertical_target_distance;

	Vector2 orbitational_acceleration;
	Vector2 orbitational_velocity;
	Vector2 orbitational_target_velocity; // target as in intended velocity

	float offset_angle;

	float offset_acceleration;
	float offset_speed;
	int offset_direction;

	float field_of_view;

	float zoom_acceleration;
	float zoom_speed;
	int zoom_direction;

	float near_clipping;
	float far_clipping;

	uint8_t cam_mode;
	float lerpTime;
	float camTime;

	CameraSettings settings;
} Camera;

// functions prototypes

Camera camera_create();
void camera_getOrbitalPosition(Camera *camera, Vector3 barycenter, float frame_time);
void camera_set(Camera *camera, Screen *screen);

// function implementations

Camera camera_create()
{
	Camera camera = {
		.position = (Vector3){0, -1200, 1200},
		.distance_from_barycenter = 500,
		.target_distance = 200,
		.angle_around_barycenter = 0,
		.pitch = 25,
		.offset_angle = 0,
		.offset_height = 250,
		.field_of_view = 80,
		.near_clipping = 100,
		.far_clipping = 3000,
		.cam_mode = 0,
		.lerpTime = 6.1f,
		.camTime = 0,
	};

	return camera;
}

void camera_getOrbitalPosition(Camera *camera, Vector3 barycenter, float frame_time)
{
	camera->orbitational_velocity.x += camera->orbitational_acceleration.x * frame_time;
	camera->orbitational_velocity.y += camera->orbitational_acceleration.y * frame_time;
	camera->zoom_speed += camera->zoom_acceleration * frame_time;
	camera->offset_speed += camera->offset_acceleration * frame_time;

	if (fabsf(camera->orbitational_velocity.x) < 1.0f && fabsf(camera->orbitational_velocity.y) < 1.0f && fabsf(camera->zoom_speed) < 1.0f && fabsf(camera->offset_speed) < 1.0f)
	{
		camera->orbitational_velocity.x = 0;
		camera->orbitational_velocity.y = 0;
		camera->zoom_speed = 0;
		camera->offset_speed = 0;
	}

	camera->pitch += camera->orbitational_velocity.x * frame_time;
	camera->angle_around_barycenter += camera->orbitational_velocity.y * frame_time;

	camera->field_of_view += camera->zoom_direction * camera->zoom_speed * frame_time;
	camera->offset_angle += camera->offset_direction * camera->offset_speed * frame_time;

	if (camera->angle_around_barycenter > 360)
		camera->angle_around_barycenter -= 360;
	if (camera->angle_around_barycenter < 0)
		camera->angle_around_barycenter += 360;

	if (camera->pitch > camera->settings.max_pitch)
		camera->pitch = camera->settings.max_pitch;
	if (camera->pitch < -camera->settings.max_pitch + 30)
		camera->pitch = -camera->settings.max_pitch + 30; // this hard coded + 20 is for the near plane to not enter the actor geometry during "camera collision"

	camera->horizontal_barycenter_distance = camera->distance_from_barycenter * fm_cosf(rad(camera->pitch));
	camera->vertical_barycenter_distance = camera->distance_from_barycenter * fm_sinf(rad(camera->pitch));

	camera->horizontal_target_distance = camera->target_distance * fm_cosf(rad(camera->pitch));
	camera->vertical_target_distance = camera->target_distance * fm_sinf(rad(camera->pitch + 180));

	camera->position.x = barycenter.x - (camera->horizontal_barycenter_distance * fm_sinf(rad(camera->angle_around_barycenter - camera->offset_angle)));
	camera->position.y = barycenter.y - (camera->horizontal_barycenter_distance * fm_cosf(rad(camera->angle_around_barycenter - camera->offset_angle)));
	camera->position.z = barycenter.z + camera->offset_height + camera->vertical_barycenter_distance;

	/* this is a temporary brute force abomination to "collide" the camera with an horizontal plane at height 20 simulating the floor,
	will be modyfied when camera collision happens */
	/*
	 */
	camera->distance_from_barycenter = camera->settings.distance_from_baricenter;
	while (camera->position.z < 30)
	{
		camera->distance_from_barycenter--;
		camera->horizontal_barycenter_distance = camera->distance_from_barycenter * fm_cosf(rad(camera->pitch));
		camera->vertical_barycenter_distance = camera->distance_from_barycenter * fm_sinf(rad(camera->pitch));

		camera->position.x = barycenter.x - camera->horizontal_barycenter_distance * fm_sinf(rad(camera->angle_around_barycenter - camera->offset_angle));
		camera->position.y = barycenter.y - camera->horizontal_barycenter_distance * fm_cosf(rad(camera->angle_around_barycenter - camera->offset_angle));
		camera->position.z = barycenter.z + camera->offset_height + camera->vertical_barycenter_distance;
	}

	camera->target.x = barycenter.x - camera->horizontal_target_distance * fm_sinf(rad(camera->angle_around_barycenter + 180));
	camera->target.y = barycenter.y - camera->horizontal_target_distance * fm_cosf(rad(camera->angle_around_barycenter + 180));
	camera->target.z = barycenter.z + camera->offset_height + camera->vertical_target_distance;
}

void camera_set(Camera *camera, Screen *screen)
{
	t3d_viewport_set_projection(
		&screen->gameplay_viewport,
		T3D_DEG_TO_RAD(camera->field_of_view),
		camera->near_clipping,
		camera->far_clipping);

	t3d_viewport_look_at(
		&screen->gameplay_viewport,
		&(T3DVec3){{camera->position.x, camera->position.y, camera->position.z}},
		&(T3DVec3){{camera->target.x, camera->target.y, camera->target.z}},
		&(T3DVec3){{0, 0, 1}});
}

#endif