//
//  main.m
//  oldiconutil
//
//  Created by Uli Kusterer on 9/5/12.
//  Copyright (c) 2012 Uli Kusterer. All rights reserved.
//

#import <Cocoa/Cocoa.h>


#define JUST_PASS_THROUGH		0
#define FILTER_TOC_OUT			1


#define SYNTAX				"oldiconutil {--help|[--inplace [--compression <compression>]|--list] <icnsFilePath>}"
#define SUMMARY				"Convert a .icns icon file holding PNG-encoded icons (supported\nin 10.6) to JPEG 2000-encoded icons (supported in 10.5)."
#define PARAMDESCRIPTIONS	"--help - Show this message.\n" \
							"icnsFilePath - Path of input icns file. Output file will have _10_5 appended to its name, unless the --inplace option is given, in which case it'll replace the input file. If --list is given, oldiconutil will simply print a description of the file.\n" \
							"compression - One of the compression formats of tif, bmp, gif, jpg, png, jp2, immediately followed by a number from 0.0 (best compression) through 1.0 (no compression) indicating how much to compress. If you do not provide a format, the default is jp2 (JPEG 2000), if you do not specify a compression factor, it defaults to 1.0 (uncompressed). Note not all formats may be recognized by Mac OS X Finder (especially in 10.5), but are provided for people who want to experiment.\n"


int main(int argc, const char * argv[])
{
	if( argc < 2 )
	{
		fprintf( stderr, "Error: Syntax is " SYNTAX "\n" );
		return 1;
	}
	
	BOOL					convertInPlace = NO;
	BOOL					listOnly = NO;
	int						nameArgumentPosition = 1;
	NSNumber*				jpegCompressionObj = [NSNumber numberWithFloat: 1.0];
	NSBitmapImageFileType	compressionType = NSJPEG2000FileType;
	NSString*				destCompression = @"jp2";
	
	if( strcasecmp( argv[1], "--help" ) == 0 )
	{
		printf( "Syntax: " SYNTAX "\n" SUMMARY "\n\n" PARAMDESCRIPTIONS "\n\n(c) 2012 by Elgato Systems GmbH, all rights reserved.\n" );
		return 0;
	}
	else if( strcasecmp( argv[1], "--inplace" ) == 0 )
	{
		convertInPlace = YES;
		nameArgumentPosition ++;

		if( argc < (nameArgumentPosition +1) )
		{
			fprintf( stderr, "Error: Syntax is " SYNTAX "\n" );
			return 4;
		}
	}
	else if( strcasecmp( argv[1], "--list" ) == 0 )
	{
		listOnly = YES;
		nameArgumentPosition ++;

		if( argc < (nameArgumentPosition +1) )
		{
			fprintf( stderr, "Error: Syntax is " SYNTAX "\n" );
			return 4;
		}
	}
	
	if( strcasecmp( argv[nameArgumentPosition], "--compression" ) == 0 )
	{
		nameArgumentPosition ++;
		
		if( argc < (nameArgumentPosition +2) )
		{
			fprintf( stderr, "Error: Syntax is " SYNTAX "\n" );
			return 4;
		}
		
		NSString*		compStr = [[NSString stringWithUTF8String: argv[nameArgumentPosition] ] lowercaseString];
		nameArgumentPosition++;
		
		// Find compression prefix (if available) and remove it so only number is left:
		NSDictionary	*compressionTypes = [NSDictionary dictionaryWithObjectsAndKeys: [NSNumber numberWithInteger: NSTIFFFileType], @"tif",
											 [NSNumber numberWithInteger: NSBMPFileType], @"bmp",
											 [NSNumber numberWithInteger: NSGIFFileType], @"gif",
											 [NSNumber numberWithInteger: NSJPEGFileType], @"jpg",
											 [NSNumber numberWithInteger: NSPNGFileType], @"png",
											 [NSNumber numberWithInteger: NSJPEG2000FileType], @"jp2", nil];
		for( NSString* prefix in compressionTypes.allKeys )
		{
			if( [compStr hasPrefix: prefix] )
			{
				destCompression = prefix;
				compressionType = [[compressionTypes objectForKey: prefix] integerValue];
				compStr = [compStr substringFromIndex: prefix.length];
				break;
			}
		}
		
		// If a compression level has been specified, parse it from the remaining string and use it:
		if( compStr.length > 0 )
		{
			float			theCompression = [compStr floatValue];
			jpegCompressionObj = [NSNumber numberWithFloat: theCompression];
		}
	}

	@autoreleasepool
	{
		NSString		*	inputPath = [NSString stringWithUTF8String: argv[nameArgumentPosition]];
		NSString		*	outputPath = convertInPlace ? inputPath : [[inputPath stringByDeletingPathExtension] stringByAppendingString: @"_10_5.icns"];
		BOOL				isDirectory = NO;
	    
		if( !inputPath || ![[NSFileManager defaultManager] fileExistsAtPath: inputPath isDirectory: &isDirectory] || isDirectory )
		{
			fprintf( stderr, "Error: Can't find input file.\n" );
			return 2;
		}
		
		NSData			*	inputData = [NSData dataWithContentsOfFile: inputPath];
		if( !inputData )
		{
			fprintf( stderr, "Error: Can't load input file.\n" );
			return 3;
		}
		
		NSMutableData	*	outputData = [NSMutableData dataWithLength: 0];
		const char* theBytes = [inputData bytes];
		NSUInteger	currOffs = 4;	// Skip 'icns'
		uint32_t	fileSize = NSSwapInt( *(uint32_t*)(theBytes +currOffs) );
		currOffs += 4;
		
		while( currOffs < fileSize )
		{
			@autoreleasepool
			{
				char		blockType[5] = { 0 };
				memmove( blockType, theBytes +currOffs, 4 );
				currOffs += 4;
				
				printf( "Found block '%s'\n", blockType );
				
#if FILTER_TOC_OUT
				if( strcmp(blockType,"TOC ") == 0 )
				{
					if( !listOnly )
					{
						uint32_t	blockSize = NSSwapInt( *(uint32_t*)(theBytes +currOffs) );
						printf( "\tSkipping %d (+4) bytes.\n", blockSize );
						currOffs += blockSize -4;
					}
				}
				else
#endif
				{
					[outputData appendBytes: blockType length: 4];	// Copy the type.
					uint32_t	blockSize = NSSwapInt( *(uint32_t*)(theBytes +currOffs) );
					currOffs += 4;
					NSData	*	currBlockData = [NSData dataWithBytes: theBytes +currOffs length: blockSize -8];
					currOffs += blockSize -8;
					uint32_t	startLong = *(uint32_t*)[currBlockData bytes];
					BOOL		shouldConvert = (startLong == 0x474E5089);	// PNG data starts with 'âPNG'.
					
					if( !listOnly )
					{
						if( !shouldConvert || strcmp(blockType,"ic08") == 0 || strcmp(blockType,"ic10") == 0
						   || strcmp(blockType,"ic13") == 0 || strcmp(blockType,"ic09") == 0 || strcmp(blockType,"ic12") == 0
						   || strcmp(blockType,"ic07") == 0 || strcmp(blockType,"ic11") == 0 || strcmp(blockType,"ic14") == 0 )
							;
						else
							shouldConvert = NO;
					}
#if JUST_PASS_THROUGH
					shouldConvert = NO;
#endif
					
					if( shouldConvert )
					{
						if( !listOnly )
						{
							printf( "\tConverting PNG to %s\n", [destCompression UTF8String] );
							
							NSBitmapImageRep	*	theImage = [[NSBitmapImageRep alloc] initWithData: currBlockData];
							NSData				*	jp2Data = [theImage representationUsingType: NSJPEG2000FileType properties:
								[NSDictionary dictionaryWithObject: jpegCompressionObj forKey:NSImageCompressionFactor]];
							uint32_t				newSize = NSSwapInt( (uint32_t) [jp2Data length] + 8 );
							[outputData appendBytes: &newSize length: 4];	// Write size.
							[outputData appendData: jp2Data];
						}
						else
							printf( "\tData is PNG\n" );
					}
					else
					{
						if( !listOnly )
						{
							printf( "\tCopying data verbatim.\n" );
							blockSize = NSSwapInt( blockSize );
							[outputData appendBytes: &blockSize length: 4];	// Copy size.
							[outputData appendData: currBlockData];
						}
						else
						{
							printf( "\tData is RLE or JPEG\n" );
						}
					}
				}
			}
		}
		
		if( !listOnly )
		{
			[outputData replaceBytesInRange: NSMakeRange(0,0) withBytes: "icns" length: 4];
			uint32_t theSize = NSSwapInt( (uint32_t)[outputData length] +4 );
			[outputData replaceBytesInRange: NSMakeRange(4,0) withBytes: &theSize length: 4];
			 
			printf( "Writing out %ld bytes.\n", [outputData length] );
			[outputData writeToFile: outputPath atomically: NO];
		}
	}
    return 0;
}

