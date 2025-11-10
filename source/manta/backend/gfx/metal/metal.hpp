#pragma once
#ifdef __OBJC__

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <QuartzCore/QuartzCore.h>

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