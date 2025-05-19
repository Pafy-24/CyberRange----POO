import { NextResponse } from 'next/server';
import { videos } from '@/data/videos';

export async function GET(request: Request) {
  const { searchParams } = new URL(request.url);
  const query = searchParams.get('q');
  
  if (!query) {
    return NextResponse.json(
      { error: 'Search query is required' },
      { status: 400 }
    );
  }
  
  try {
    // Convert query to lowercase for case-insensitive search
    const searchQuery = query.toLowerCase();
    
    console.log("Search query:", searchQuery);
    console.log("Videos count:", videos.length);
    
    // Log sample titles for debugging
    console.log("Sample video titles:", videos.slice(0, 3).map(v => v.title));
    
    // Search in title, channel name, and category
    const searchResults = videos.filter(video => {
      const titleMatch = video.title.toLowerCase().includes(searchQuery);
      const channelMatch = video.channelName.toLowerCase().includes(searchQuery);
      const categoryMatch = video.category.toLowerCase().includes(searchQuery);
      
      // Log matches for debugging
      if (titleMatch || channelMatch || categoryMatch) {
        console.log(`Match found: "${video.title}" - Title match: ${titleMatch}, Channel match: ${channelMatch}, Category match: ${categoryMatch}`);
      }
      
      return titleMatch || channelMatch || categoryMatch;
    });
    
    console.log(`Found ${searchResults.length} results for query "${query}"`);
    
    return NextResponse.json({
      results: searchResults,
      count: searchResults.length,
      query: query
    }, { status: 200 });
  } catch (error) {
    console.error('Search error:', error);
    return NextResponse.json(
      { error: 'Failed to execute search' },
      { status: 500 }
    );
  }
} 