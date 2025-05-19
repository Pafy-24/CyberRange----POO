import { NextResponse } from 'next/server';
import { videos } from '@/data/videos';

export async function GET(request: Request) {
  // Get URL to parse query parameters
  const { searchParams } = new URL(request.url);
  const category = searchParams.get('category');
  
  try {
    // If category is provided, filter videos by category
    const filteredVideos = category 
      ? videos.filter(video => video.category === category)
      : videos;
      
    return NextResponse.json({
      videos: filteredVideos,
      totalCount: filteredVideos.length
    }, { status: 200 });
  } catch (error) {
    return NextResponse.json(
      { error: 'Failed to fetch videos' },
      { status: 500 }
    );
  }
} 