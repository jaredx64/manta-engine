#include <manta/skeleton.hpp>

#include <vendor/new.hpp>

#include <core/memory.hpp>

#include <manta/draw.hpp>
#include <manta/assets.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void EasedFloat::update( const Delta delta )
{
	if( easeCurrent < 0.0f ) { easeCurrent = 0.0f; return; }
	easeCurrent -= delta * easeFactor;
}


void EasedFloat::set_value( const float value, const float ease )
{
	valueStart = get_value();
	valueTarget = value;
	easeCurrent = 1.0f;
	easeFactor = 1.0f / ease;
}


float EasedFloat::get_value() const
{
	return easeCurrent > 0.0f ? lerp( valueTarget, valueStart, easeCurrent ) : valueTarget;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr float ANIMATION_SPEED = 1.0;
constexpr u16 ANIMATION_KEYFRAMES = 8;


float Animation::get_value( const Timeline timeline ) const
{
	const BinAnimation &binAnimation = binAnimations[index];
	if( binAnimation.keyframeCount[timeline] == 0 ) { return 0.0f; }

	const int keyframeCurrent = static_cast<int>( time * binAnimation.keyframeCount[timeline] );
	const u32 indexCurrent = binAnimation.keyframeFirst[timeline] + keyframeCurrent;
	const BinKeyframe &frameCurrent = binKeyframes[indexCurrent];

	const int keyframeNext = ( ( keyframeCurrent + 1 ) % binAnimation.keyframeCount[timeline] );
	const u32 indexNext = binAnimation.keyframeFirst[timeline] + keyframeNext;
	const BinKeyframe &frameNext = binKeyframes[indexNext];

	const float a = time - frameCurrent.time + ( time < frameCurrent.time ) * 1.0f;
	const float b = frameNext.time - frameCurrent.time + ( frameNext.time < frameCurrent.time ) * 1.0f;
	const float progress = a / b;

	return lerp( frameCurrent.value, frameNext.value, progress );
}


float Animation::get_rotation( const Timeline timeline ) const
{
	const BinAnimation &binAnimation = binAnimations[index];
	if( binAnimation.keyframeCount[timeline] == 0 ) { return 0.0f; }

	const int keyframeCurr = static_cast<int>( time * binAnimation.keyframeCount[timeline] );
	const u32 indexCurr = binAnimation.keyframeFirst[timeline] + keyframeCurr;
	const BinKeyframe &frameCurr = binKeyframes[indexCurr];

	const int keyframeNext = ( ( keyframeCurr + 1 ) % binAnimation.keyframeCount[timeline] );
	const u32 indexNext = binAnimation.keyframeFirst[timeline] + keyframeNext;
	const BinKeyframe &frameNext = binKeyframes[indexNext];

	const float a = time - frameCurr.time + ( time < frameCurr.time ) * 1.0f;
	const float b = frameNext.time - frameCurr.time + ( frameNext.time < frameCurr.time ) * 1.0f;
	const float progress = a / b;

	return lerp_angle_degrees( frameCurr.value, frameNext.value, progress );
}


void Animation::update( const Delta delta )
{
	const BinAnimation &binAnimation = binAnimations[index];
	const Delta dt = ( delta * speed ) / binAnimation.duration;// * ANIMATION_SPEED * speed;

	time += static_cast<float>( dt );

	if( time > 1.0f )
	{
		if( !loop ) { index = U32_MAX; time = 0.0f; return; }
		time = static_cast<float>( fmod( time, 1.0 ) );
	}

	weight.update( dt );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Attachment::set_sprite( const int sprite, int subimg )
{
	this->sprite = sprite;
	this->subimg = subimg;
}


void Attachment::set_color( const Color color )
{
	this->color = color;
}


void Attachment::set_scale( const float scale )
{
	this->scaleX = scale;
	this->scaleY = scale;
}


void Attachment::set_rotation( const float angleDegrees )
{
	this->rotation = angleDegrees;
}


void Attachment::set_translation( const float x, const float y )
{
	this->x = x;
	this->y = y;
}


void Attachment::draw( const Bone &bone, const Delta delta, float drawX, float drawY, Color drawColor, float depth )
{
	const float cosParent = cosf( bone.rotation * DEG2RAD_F );
    const float sinParent = sinf( bone.rotation * DEG2RAD_F );

	// Scale
	const float dScaleX = scaleX * bone.scaleX;
	const float dScaleY = scaleY * bone.scaleY;

	// Translation
	const float dX = bone.x + ( x * dScaleX * cosParent - y * dScaleX * sinParent );
    const float dY = bone.y + ( x * dScaleY * sinParent + y * dScaleY * cosParent );

	// Rotation
	const float dRotation = rotation + bone.rotation;

	// Color
	const Color dColor = color * bone.color;

	// Draw Sprite
	draw_sprite_angle( sprite, subimg, drawX + dX, drawY + dY, dRotation,
		dScaleX, dScaleY, drawColor * dColor, depth );

	draw_circle( drawX + dX, drawY + dY, 8, c_lime );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Bone::animation_start( AnimationID animation, bool loop, float weight, float ease )
{
	Assert( animation < BONE_ANIMATION_COUNT );
	animations[animation].index = animation;
	animations[animation].weight.set_value( weight, ease );
	animations[animation].loop = loop;
	animations[animation].speed = 1.0f;
	return true;
}


void Bone::animation_stop( AnimationID animation, bool ease )
{
	Assert( animation < BONE_ANIMATION_COUNT );
}


void Bone::animation_stop_all()
{
}


void Bone::animation_weight( AnimationID animation, float weight, float ease )
{
	Assert( animation < BONE_ANIMATION_COUNT );
	animations[animation].weight.set_value( weight, ease );
}


void Bone::animation_speed( AnimationID animation, float speed )
{
	Assert( animation < BONE_ANIMATION_COUNT );
	animations[animation].speed = speed;
}


void Bone::animation_time( AnimationID animation, float time )
{
	Assert( animation < BONE_ANIMATION_COUNT );
	animations[animation].time = time;
}


bool Bone::attachment_clear( const u8 attachmentIndex )
{
	Assert( attachmentIndex < BONE_ATTACHMENTS_COUNT );
	if( attachments[attachmentIndex].sprite == 0 ) { return false; }
	new ( &attachments[attachmentIndex] ) Attachment();
	return true;
}


Attachment &Bone::get_attachment( const u8 attachmentIndex )
{
	Assert( attachmentIndex < BONE_ATTACHMENTS_COUNT );
	return attachments[attachmentIndex];
}


void Bone::set_color( const Color color )
{
	customColor = color;
}


Color Bone::get_color() const
{
	return color;
}


void Bone::set_scale( const float scaleX, const float scaleY )
{
	customScaleX = scaleX;
	customScaleY = scaleY;
}


float Bone::get_scale_x() const
{
	return scaleX;
}


float Bone::get_scale_y() const
{
	return scaleY;
}


void Bone::set_rotation( const float angle )
{
	customRotation = angle;
}


float Bone::get_rotation() const
{
	return rotation;
}


void Bone::set_translation( const float x, const float y )
{
	customX = x;
	customY = y;
}


float Bone::get_translation_x() const
{
	return x;
}


float Bone::get_translation_y() const
{
	return y;
}


void Bone::update( const BinBone &binBone, const Delta delta )
{
	// Fetch Parent
	const bool hasParent = ( binBone.parent != U32_MAX );
	const float parentX = ( hasParent ? skeleton->bones[binBone.parent].x : 0.0f );
	const float parentY = ( hasParent ? skeleton->bones[binBone.parent].y : 0.0f );
	const float parentLength = ( hasParent ? skeleton->bones[binBone.parent].length : 0.0f );
	const float parentScaleX = ( hasParent ? skeleton->bones[binBone.parent].scaleX : 1.0f );
	const float parentScaleY = ( hasParent ? skeleton->bones[binBone.parent].scaleY : 1.0f );
	const float parentRotation = ( hasParent ? skeleton->bones[binBone.parent].rotation : 0.0f );
	const Color parentColor = ( hasParent ? skeleton->bones[binBone.parent].color : c_white );

	float animRotation = 0.0f;
	float animRotX = 0.0f;
	float animRotY = 0.0f;
	float animX = 0.0f;
	float animY = 0.0f;
	float animScaleX = 1.0f;
	float animScaleY = 1.0f;
	float animShearX = 1.0f;
	float animShearY = 1.0f;
	float animWeight = 0.0f;

	// Update Animations
	for( u8 i = 0; i < BONE_ANIMATION_COUNT; i++ )
	{
		Animation &animation = animations[i];
		if( animations[i].index == U32_MAX ) { continue; }

		animation.update( delta );

		const float weight = animation.weight.get_value();
		const float rotation = animation.get_rotation( Timeline_Rotation );

		animRotX += weight * cosf( rotation * DEG2RAD_F );
		animRotY += weight * sinf( rotation * DEG2RAD_F );
		animX += weight * animation.get_value( Timeline_TranslationX );
		animY += weight * animation.get_value( Timeline_TranslationY );
		animScaleX += weight * animation.get_value( Timeline_ScaleX );
		animScaleY += weight * animation.get_value( Timeline_ScaleY );
		animShearX += weight * animation.get_value( Timeline_ShearX );
		animShearY += weight * animation.get_value( Timeline_ShearY );
		animWeight += weight;
	}

	// Average Animation Transformations
	if( animWeight > 0.0f )
	{
		const float magnitude = sqrt( animRotX * animRotX + animRotY * animRotY );
		animRotX /= magnitude;
		animRotY /= magnitude;
		animRotation = atan2f( animRotX, animRotY ) * RAD2DEG_F;
		animX /= animWeight;
		animY /= animWeight;
		animScaleX /= animWeight;
		animScaleY /= animWeight;
		animShearX /= animWeight;
		animShearY /= animWeight;
	}

	// Scale
	scaleX = customScaleX * binBone.scaleX * animScaleX * parentScaleX;
	scaleY = customScaleY * binBone.scaleY * animScaleY * parentScaleY;

	// Translation
	const float localX = binBone.x + animX;
	const float localY = binBone.y + animY;
	const float cosParent = cosf( parentRotation * DEG2RAD_F );
    const float sinParent = sinf( parentRotation * DEG2RAD_F );
	x = ( parentX + lengthdir_x( parentLength * parentScaleX, parentRotation ) ) +
		( localX * scaleX * cosParent - localY * scaleX * sinParent );
	y = ( parentY + lengthdir_y( parentLength * parentScaleY, parentRotation ) ) +
		( localX * scaleY * sinParent + localY * scaleY * cosParent );

	// Rotation
	rotation = customRotation + binBone.rotation + animRotation + parentRotation;

	// Color
	color = color * customColor * parentColor;
}


void Bone::draw( const Delta delta, float drawX, float drawY, Color drawColor, float depth )
{
	const float x1 = x + drawX;
	const float y1 = y + drawY;
	const float x2 = x1 + lengthdir_x( length * scaleX, rotation );
	const float y2 = y1 + lengthdir_y( length * scaleY, rotation );

	draw_line( x1, y1, x2, y2, color * drawColor, 1.0 );
	draw_circle( x1, y1, scaleX, c_red );
	draw_circle( x2, y2, scaleX, c_blue );

	draw_text_f( fnt_iosevka, 12, x1, y1 + 16.0f, c_white, "%d", id );
	draw_text_f( fnt_iosevka, 12, x2, y2 - 24.0f, c_green, "%d", id );

	// Draw Attachments
	for( u8 i = 0; i < BONE_ATTACHMENTS_COUNT; i++ )
	{
		Attachment &attachment = attachments[i];
		if( attachment.sprite == 0 ) { continue; }
		attachment.draw( *this, delta, drawX, drawY, drawColor, depth );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Skeleton::init( SkeletonID type )
{
	Assert( type < numSkeletons );
	this->type = type;
	BinSkeleton &binSkeleton = binSkeletons[this->type];

	// Allocate Memory
	MemoryAssert( bones == nullptr );
	bones = reinterpret_cast<Bone *>( memory_alloc( binSkeleton.boneCount * sizeof( Bone ) ) );

	// Setup Bones
	for( u32 i = 0; i < binSkeleton.boneCount; i++ )
	{
		new ( &bones[i] ) Bone();
		Bone &bone = bones[i];
		bone.skeleton = this;
		bone.id = i;

		const BinBone &binBone = binBones[binSkeleton.boneFirst + i];
		bone.length = binBone.length;
		bone.customScaleX = 1.0f;
		bone.customScaleY = 1.0f;
		bone.color = c_white;
		bone.customColor = c_white;
	}

	return true;
}

bool Skeleton::free()
{
	// Free Memory
	MemoryAssert( bones != nullptr );
	memory_free( bones );
	bones = nullptr;
	return true;
}

Bone &Skeleton::get_bone( const BoneID boneID )
{
	Assert( boneID < binSkeletons[type].boneCount );
	MemoryAssert( bones != nullptr );
	return bones[boneID];
}


bool Skeleton::animation_start( AnimationID animation, bool loop, float weight, float ease )
{
	MemoryAssert( bones != nullptr );
	return bones[0].animation_start( animation, loop, weight, ease );
}


void Skeleton::animation_stop( AnimationID animation, bool ease )
{
	MemoryAssert( bones != nullptr );
	bones[0].animation_stop( animation, ease );
}


void Skeleton::animation_stop_all()
{
	MemoryAssert( bones != nullptr );
	bones[0].animation_stop_all();
}


void Skeleton::animation_weight( AnimationID animation, float weight, float ease )
{
	MemoryAssert( bones != nullptr );
	bones[0].animation_weight( weight, ease );
}


void Skeleton::animation_speed( AnimationID animation, float speed )
{
	MemoryAssert( bones != nullptr );
	bones[0].animation_time( animation, speed );
}


void Skeleton::animation_time( AnimationID animation, float time )
{
	MemoryAssert( bones != nullptr );
	bones[0].animation_time( animation, time );
}


void Skeleton::set_color( const Color color )
{
	MemoryAssert( bones != nullptr );
	bones[0].set_color( color );
}


void Skeleton::set_scale( const float scaleX, const float scaleY )
{
	MemoryAssert( bones != nullptr );
	bones[0].set_scale( scaleX, scaleY );
}


void Skeleton::set_rotation( const float angle )
{
	MemoryAssert( bones != nullptr );
	bones[0].set_rotation( angle );
}


void Skeleton::set_translation( const float x, const float y )
{
	MemoryAssert( bones != nullptr );
	bones[0].set_translation( x, y );
}


void Skeleton::update( const Delta delta )
{
	MemoryAssert( bones != nullptr );
	BinSkeleton &binSkeleton = binSkeletons[this->type];

	for( u32 i = 0; i < binSkeleton.boneCount; i++ )
	{
		const BinBone &binBone = binBones[binSkeleton.boneFirst + i];
		Bone &bone = bones[i];
		bones[i].update( binBone, delta );
	}
}


void Skeleton::draw( const Delta delta, float x, float y, Color color, float depth )
{
	MemoryAssert( bones != nullptr );
	BinSkeleton &binSkeleton = binSkeletons[this->type];

	for( u32 i = 0; i < binSkeleton.boneCount; i++ )
	{
		bones[i].draw( delta, x, y, color, depth );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////