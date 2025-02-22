#pragma once

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/color.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr int BONE_ANIMATION_COUNT = 4;
constexpr int BONE_ATTACHMENTS_COUNT = 4;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class EasedFloat
{
public:
	EasedFloat() { };
	EasedFloat( const float value ) { set_value( value, 0.0 ); }
	EasedFloat( const float value, const float ease ) { set_value( value, ease ); }

	void update( const Delta delta );
	void set_value( const float value, const float ease = 0.0f );
	float get_value() const;

private:
	float valueStart = 0.0f;
	float valueTarget = 0.0f;
	float easeCurrent = 0.0f;
	float easeFactor = 0.0f;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DiskKeyframe
{
	float time;
	float value;
};

constexpr int numKeyframes = 14;
static DiskKeyframe diskKeyframes[numKeyframes] =
{
	{ .time = 0.00f, .value = 0.0f },
	{ .time = 0.25f, .value = 90.0f },
	{ .time = 0.50f, .value = 180.0f },
	{ .time = 0.75f, .value = -90.0f },

	{ .time = 0.00f, .value = 0.0f },
	{ .time = 0.25f, .value = 30.0f },
	{ .time = 0.50f, .value = 0.0f },
	{ .time = 0.75f, .value = 330.0f },

	{ .time = 0.00f, .value = -4.0f },
	{ .time = 0.33f, .value = 0.0f },
	{ .time = 0.66f, .value = 4.0f },

	{ .time = 0.00f, .value = 4.0f },
	{ .time = 0.33f, .value = -4.0f },
	{ .time = 0.66f, .value = 4.0f },
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( Timeline, int )
{
	Timeline_Rotation,
	Timeline_TranslationX,
	Timeline_TranslationY,
	Timeline_ScaleX,
	Timeline_ScaleY,
	Timeline_ShearX,
	Timeline_ShearY,
	TIMELINE_COUNT,
};


struct DiskAnimation
{
	u32 keyframeFirst[TIMELINE_COUNT];
	u32 keyframeCount[TIMELINE_COUNT];
	float duration;
};


constexpr int numAnimations = 2;
static DiskAnimation diskAnimations[numAnimations] =
{
	{ .keyframeFirst = { 0, 0, 0, 0, 0, 0, 0 }, .keyframeCount = { 4, 0, 0, 0, 0, 0, 0 }, .duration = 1.0f },

	{ .keyframeFirst = { 4, 8, 11, 0, 0, 0, 0 }, .keyframeCount = { 4, 3, 3, 0, 0, 0, 0 }, .duration = 1.0f },
};


enum_type( AnimationID, u32 )
{
	AnimationID_Test_Anim0 = 0,
	// ...
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DiskBone
{
	u32 animationFirst;
	u32 animationCount;
	u32 parent;

	float x;
	float y;
	float length;
	float scaleX;
	float scaleY;
	float rotation;
};


enum_type( BoneID, u32 )
{
	BoneID_Test_Root = 0,
	// ...
};

constexpr int numBones = 4;
static DiskBone diskBones[numBones] =
{
	{ .animationFirst = 0, .animationCount = 1, .parent = U32_MAX,
		.x = 0.0f, .y = 0.0f, .length = 16.0f, .scaleX = 1.0f, .scaleY = 1.0f, .rotation = 0.0f },

	{ .animationFirst = 0, .animationCount = 1, .parent = 0,
		.x = 0.0f, .y = 0.0f, .length = 8.0f, .scaleX = 1.0f, .scaleY = 1.0f, .rotation = -45.0f },

	{ .animationFirst = 0, .animationCount = 1, .parent = 0,
		.x = 0.0f, .y = 0.0f, .length = 8.0f, .scaleX = 1.0f, .scaleY = 1.0f, .rotation = 45.0f },

	{ .animationFirst = 0, .animationCount = 1, .parent = 2,
		.x = 1.0f, .y = 1.0f, .length = 10.0f, .scaleX = 2.0f, .scaleY = 2.0f, .rotation = 90.0f },
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DiskSkeleton
{
	u32 boneFirst;
	u32 boneCount;
};


enum_type( SkeletonID, u32 )
{
	SkeletonID_Test = 0,
	// ...
};


constexpr int numSkeletons = 1;
static DiskSkeleton diskSkeletons[numSkeletons] =
{
	{ .boneFirst = 0, .boneCount = 4 },
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Animation
{
public:
	void update( const Delta delta );

	float get_value( const Timeline timeline ) const;
	float get_rotation( const Timeline timeline ) const;

private:
	friend class Bone;
	u32 index = U32_MAX;
	bool loop = false;
	bool unused = false;
	float time = 0.0;
	float speed = 0.0;
	EasedFloat weight = EasedFloat { 1.0, 0.0 };
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Attachment
{
public:
	void set_sprite( const int sprite, int subimg );
	void set_color( const Color color );
	void set_scale( const float scale );
	void set_rotation( const float angleDegrees );
	void set_translation( const float x, const float y );

	void draw( const class Bone &bone, const Delta delta, float x, float y, Color color, float depth = 0.0f );

private:
	friend class Bone;
	u32 sprite = 0;
	u32 subimg = 0;
	Color color = c_white;
	float x = 0.0f;
	float y = 0.0f;
	float scaleX = 1.0f;
	float scaleY = 1.0f;
	float rotation = 0.0f;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Bone
{
public:
	bool animation_start( AnimationID animation, bool loop = true, float weight = 1.0, float ease = 0.0f );
	void animation_stop( AnimationID animation, bool ease = 0.0f );
	void animation_stop_all();
	void animation_weight( AnimationID animation, float weight );
	void animation_speed( AnimationID animation, float speed );
	void animation_time( AnimationID animation, float time );

	int attachment_add();
	bool attachment_remove( int attachmentID );
	Attachment &get_attachment( int attachmentID );

	void set_color( const Color color );
	Color get_color() const;

	void set_scale( const float scaleX, const float scaleY );
	float get_scale_x() const;
	float get_scale_y() const;

	void set_rotation( const float angleDegrees );
	float get_rotation() const;

	void set_translation( const float x, const float y );
	float get_translation_x() const;
	float get_translation_y() const;

	void update( const DiskBone &diskBone, const Delta delta );
	void draw( const Delta delta, float x, float y, Color color, float depth = 0.0f );

private:
	friend class Skeleton;
	friend class Attachment;

	class Skeleton *skeleton = nullptr;
	u32 id = U32_MAX;
	u32 parent = U32_MAX;

	Color color;
	float x;
	float y;
	float length;
	float scaleX;
	float scaleY;
	float rotation;

	Color customColor;
	float customX;
	float customY;
	float customScaleX;
	float customScaleY;
	float customRotation;

	Animation animations[BONE_ANIMATION_COUNT];
	Attachment attachments[BONE_ATTACHMENTS_COUNT];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Skeleton
{
public:
	bool init( SkeletonID type );
	bool free();

	Bone &get_bone( const BoneID boneID );

	bool animation_start( AnimationID animation, bool loop = true, float weight = 1.0, float ease = 0.0f );
	void animation_stop( AnimationID animation, bool ease = 0.0f );
	void animation_stop_all();
	void animation_weight( AnimationID animation, float weight );
	void animation_speed( AnimationID animation, float speed );
	void animation_time( AnimationID animation, float time );

	void set_color( const Color color );
	void set_scale( const float scaleX, const float scaleY );
	void set_rotation( const float angle );
	void set_translation( const float x, const float y );

	void update( const Delta delta );
	void draw( const Delta delta, float x, float y, Color color, float depth = 0.0f );

public:
	u32 type = U32_MAX;
	//u32 index = U32_MAX;
	Bone *bones = nullptr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
