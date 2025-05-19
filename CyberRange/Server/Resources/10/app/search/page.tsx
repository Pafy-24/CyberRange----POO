import { Header } from "@/components/Header";
import { Sidebar } from "@/components/Sidebar";
import { VideoGrid } from "@/components/VideoGrid";
import { Video } from "@/data/videos";
import { videos } from "@/data/videos";
import { notFound } from "next/navigation";

interface SearchPageProps {
  searchParams: { q?: string };
}

interface SearchResponse {
  results: Video[];
  count: number;
  query: string;
}

async function getSearchResults(query: string): Promise<Video[]> {
  try {
    // For debugging purposes, log the API URL
    const apiUrl = `${process.env.NEXT_PUBLIC_API_URL || ''}/api/search?q=${encodeURIComponent(query)}`;
    console.log("Fetching search results from:", apiUrl);

    const res = await fetch(apiUrl, {
      cache: 'no-store' // Disable cache to get fresh results
    });
    
    if (!res.ok) {
      console.error(`Search failed with status: ${res.status}`);
      throw new Error(`Search failed with status: ${res.status}`);
    }
    
    const data: SearchResponse = await res.json();
    console.log("Search results:", data);
    return data.results;
  } catch (error) {
    console.error('Error fetching search results:', error);
    // If API fails, return empty array so we can fall back to client-side search
    return [];
  }
}

// Fallback client-side search function
function searchVideosDirectly(query: string): Video[] {
  const searchTerm = query.toLowerCase();
  
  return videos.filter(video => 
    video.title.toLowerCase().includes(searchTerm) ||
    video.channelName.toLowerCase().includes(searchTerm) ||
    video.category.toLowerCase().includes(searchTerm)
  );
}

export default async function SearchPage({ searchParams }: SearchPageProps) {
  const query = searchParams.q?.trim();
  
  if (!query) {
    notFound();
  }
  
  // Try to get search results from API first
  let searchResults = await getSearchResults(query);
  
  // If API returned no results, try client-side search as fallback
  if (searchResults.length === 0) {
    console.log("API returned no results, trying client-side search");
    searchResults = searchVideosDirectly(query);
    console.log(`Client-side search found ${searchResults.length} results`);
  }
  
  return (
    <div className="min-h-screen bg-background">
      <Header />
      <div className="flex">
        <Sidebar />
        <div className="md:ml-60 w-full px-4 md:px-8 pt-6 pb-20">
          <h1 className="text-3xl font-bold mb-2">Search Results</h1>
          <p className="text-gray-600 dark:text-gray-400 mb-8">
            Found {searchResults.length} results for &quot;{query}&quot;
          </p>
          
          {searchResults.length > 0 ? (
            <VideoGrid videos={searchResults} />
          ) : (
            <div className="text-center py-16">
              <svg 
                className="w-16 h-16 mx-auto mb-4 text-gray-400" 
                fill="currentColor" 
                viewBox="0 0 24 24"
              >
                <path d="M15.5 14h-.79l-.28-.27C15.41 12.59 16 11.11 16 9.5 16 5.91 13.09 3 9.5 3S3 5.91 3 9.5 5.91 16 9.5 16c1.61 0 3.09-.59 4.23-1.57l.27.28v.79l5 4.99L20.49 19l-4.99-5zm-6 0C7.01 14 5 11.99 5 9.5S7.01 5 9.5 5 14 7.01 14 9.5 11.99 14 9.5 14z" />
              </svg>
              <h2 className="text-2xl font-bold mb-2">No videos found</h2>
              <p className="text-gray-500 mb-8">Try different keywords or check for typos</p>
              
              <div className="max-w-lg mx-auto">
                <h3 className="font-semibold mb-4 text-left">Search tips:</h3>
                <ul className="text-left text-gray-600 space-y-2 list-disc pl-5">
                  <li>Make sure all words are spelled correctly</li>
                  <li>Try using more general keywords</li>
                  <li>Try searching for specific crocodile species (nile, saltwater, alligator, etc.)</li>
                  <li>Try searching for channel names</li>
                  <li>Browse videos by category using the sidebar</li>
                </ul>
              </div>
            </div>
          )}
        </div>
      </div>
    </div>
  );
} 