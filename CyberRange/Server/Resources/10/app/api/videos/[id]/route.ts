import { NextResponse } from 'next/server';
import { videos } from '@/data/videos';

export async function GET(
  request: Request,
  { params }: { params: { id: string } }
) {
  const id = params.id;
  
  try {
    // Find the video with the given ID
    const video = videos.find(v => v.id === id);
    
    if (!video) {
      return NextResponse.json(
        { error: 'Video not found' },
        { status: 404 }
      );
    }
    
    // Get related videos from the same category, excluding current video
    const relatedVideos = videos
      .filter(v => v.id !== id && v.category === video.category)
      .slice(0, 5); // Limit to 5 related videos
    
    return NextResponse.json({
      video,
      relatedVideos
    }, { status: 200 });
  } catch (error) {
    return NextResponse.json(
      { error: 'Failed to fetch video' },
      { status: 500 }
    );
  }
} 