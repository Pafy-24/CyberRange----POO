import { Header } from "@/components/Header";
import { Sidebar } from "@/components/Sidebar";
import { VideoGrid } from "@/components/VideoGrid";
import { videos } from "@/data/videos";

export default function TrendingPage() {
  // Sort videos by views to get trending videos
  const trendingVideos = [...videos].sort((a, b) => b.views - a.views);
  
  return (
    <div className="min-h-screen bg-background">
      <Header />
      <div className="flex">
        <Sidebar />
        <div className="md:ml-60 w-full px-4 md:px-8 pt-6 pb-20">
          <h1 className="text-3xl font-bold mb-8">Trending Videos</h1>
          
          <div className="mb-12">
            <ul className="flex border-b border-gray-200 mb-8">
              <li className="mr-4">
                <button className="border-b-2 border-primary px-4 py-2 font-medium text-primary">
                  Now
                </button>
              </li>
              <li className="mr-4">
                <button className="px-4 py-2 text-gray-500 hover:text-gray-700">
                  This Week
                </button>
              </li>
              <li>
                <button className="px-4 py-2 text-gray-500 hover:text-gray-700">
                  This Month
                </button>
              </li>
            </ul>
            
            <VideoGrid videos={trendingVideos} />
          </div>
        </div>
      </div>
    </div>
  );
} 