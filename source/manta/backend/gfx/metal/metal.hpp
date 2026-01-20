#pragma once
#ifdef __OBJC__

#include <vendor/conflicts.hpp>
	#import <Cocoa/Cocoa.h>
	#import <Metal/Metal.h>
	#import <QuartzCore/CAMetalLayer.h>
	#import <QuartzCore/QuartzCore.h>
#include <vendor/conflicts.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct MetalInputLayoutAttributes
{
	NSUInteger offset;
	MTLVertexFormat format;
};


struct MetalInputLayoutFormats
{
	const MetalInputLayoutAttributes *attributes;
	NSUInteger attributesCount;
	NSUInteger stepStride;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif