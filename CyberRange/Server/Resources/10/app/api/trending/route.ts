import { NextResponse } from 'next/server';
import { videos } from '@/data/videos';

export async function GET(request: Request) {
  const { searchParams } = new URL(request.url);
  const limit = searchParams.get('limit') ? parseInt(searchParams.get('limit')!) : 10;
  
  try {
    // Sort videos by view count in descending order
    const trendingVideos = [...videos]
      .sort((a, b) => b.views - a.views)
      .slice(0, limit);
    
    return NextResponse.json({
      videos: trendingVideos,
      totalCount: trendingVideos.length
    }, { status: 200 });
  } catch (error) {
    return NextResponse.json(
      { error: 'Failed to fetch trending videos' },
      { status: 500 }
    );
  }
} 