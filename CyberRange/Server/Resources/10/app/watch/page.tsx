import { Header } from "@/components/Header";
import { videos } from "@/data/videos";
import { VideoGrid } from "@/components/VideoGrid";
import { VideoPlayer } from "@/components/VideoPlayer";
import Image from "next/image";
import Link from "next/link";

export default function WatchPage({
  searchParams,
}: {
  searchParams: { v: string };
}) {
  const videoId = searchParams.v;
  const video = videos.find((v) => v.id === videoId);
  
  // Get related videos (excluding current video)
  const relatedVideos = videos
    .filter((v) => v.id !== videoId && v.category === video?.category)
    .slice(0, 3);
  
  if (!video) {
    return (
      <div className="min-h-screen bg-background">
        <Header />
        <div className="container mx-auto px-4 py-8">
          <div className="text-center">
            <h1 className="text-2xl font-bold mb-4">Video not found</h1>
            <p className="mb-6">The video you're looking for doesn't exist or has been removed.</p>
            <Link href="/" className="bg-primary text-white px-4 py-2 rounded-md">
              Return to homepage
            </Link>
          </div>
        </div>
      </div>
    );
  }
  
  return (
    <div className="min-h-screen bg-background">
      <Header />
      
      <div className="container mx-auto px-4 py-6">
        <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
          <div className="lg:col-span-2">
            {/* Video Player with audio */}
            <VideoPlayer 
              thumbnailSrc={video.thumbnail}
              title={video.title}
            />
            
            {/* Video Info */}
            <div className="mb-6">
              <h1 className="text-2xl font-bold mb-2">{video.title}</h1>
              <div className="flex justify-between items-center mb-4">
                <div className="text-sm text-gray-500">
                  {video.views.toLocaleString()} views • {video.published}
                </div>
                <div className="flex gap-4">
                  <button className="flex items-center gap-2 text-sm">
                    <svg className="w-5 h-5" viewBox="0 0 24 24" fill="currentColor">
                      <path d="M1 21h4V9H1v12zm22-11c0-1.1-.9-2-2-2h-6.31l.95-4.57.03-.32c0-.41-.17-.79-.44-1.06L14.17 1 7.59 7.59C7.22 7.95 7 8.45 7 9v10c0 1.1.9 2 2 2h9c.83 0 1.54-.5 1.84-1.22l3.02-7.05c.09-.23.14-.47.14-.73v-1.91l-.01-.01L23 10z" />
                    </svg>
                    Like
                  </button>
                  <button className="flex items-center gap-2 text-sm">
                    <svg className="w-5 h-5" viewBox="0 0 24 24" fill="currentColor">
                      <path d="M15 3H6c-.83 0-1.54.5-1.84 1.22l-3.02 7.05c-.09.23-.14.47-.14.73v1.91l.01.01L1 14c0 1.1.9 2 2 2h6.31l-.95 4.57-.03.32c0 .41.17.79.44 1.06L9.83 23l6.59-6.59c.36-.36.58-.86.58-1.41V5c0-1.1-.9-2-2-2zm4 0v12h4V3h-4z" />
                    </svg>
                    Dislike
                  </button>
                  <button className="flex items-center gap-2 text-sm">
                    <svg className="w-5 h-5" viewBox="0 0 24 24" fill="currentColor">
                      <path d="M10 9V5l-7 7 7 7v-4.1c5 0 8.5 1.6 11 5.1-1-5-4-10-11-11z" />
                    </svg>
                    Share
                  </button>
                </div>
              </div>
              
              {/* Channel Info */}
              <div className="flex items-center gap-4 py-4 border-t border-b border-gray-200 dark:border-gray-800">
                <Link href={`/channel/${video.channelName.toLowerCase().replace(/\s+/g, '')}`}>
                  <Image
                    src={video.channelAvatar}
                    alt={video.channelName}
                    width={48}
                    height={48}
                    className="rounded-full"
                  />
                </Link>
                <div>
                  <Link
                    href={`/channel/${video.channelName.toLowerCase().replace(/\s+/g, '')}`}
                    className="font-medium hover:text-primary"
                  >
                    {video.channelName}
                  </Link>
                  <p className="text-sm text-gray-500">142K subscribers</p>
                </div>
                <button className="ml-auto bg-primary hover:bg-opacity-90 text-white px-4 py-2 rounded-full">
                  Subscribe
                </button>
              </div>
              
              {/* Video Description */}
              <div className="mt-4 bg-gray-50 dark:bg-gray-900 p-4 rounded-xl">
                <p className="text-sm">
                  Explore the fascinating world of {video.title.split(':')[0]} in this incredible footage.
                  Watch as these magnificent reptiles showcase their natural behaviors in their habitat.
                  <br /><br />
                  #Crocodiles #Wildlife #NatureDocumentary
                </p>
              </div>
            </div>
          </div>
          
          {/* Related Videos */}
          <div className="lg:col-span-1">
            <h3 className="text-lg font-medium mb-4">Related Videos</h3>
            <div className="space-y-4">
              {relatedVideos.map((relatedVideo) => (
                <Link 
                  href={`/watch?v=${relatedVideo.id}`} 
                  key={relatedVideo.id}
                  className="flex gap-2 group"
                >
                  <div className="flex-shrink-0 relative w-40 aspect-video rounded-lg overflow-hidden">
                    <Image
                      src={relatedVideo.thumbnail}
                      alt={relatedVideo.title}
                      fill
                      className="object-cover group-hover:scale-105 transition-transform"
                    />
                    <div className="absolute bottom-1 right-1 bg-black bg-opacity-80 text-white text-xs px-1 rounded">
                      {relatedVideo.duration}
                    </div>
                  </div>
                  <div className="flex-1">
                    <h4 className="text-sm font-medium line-clamp-2 group-hover:text-primary transition-colors">
                      {relatedVideo.title}
                    </h4>
                    <p className="text-xs text-gray-500 mt-1">{relatedVideo.channelName}</p>
                    <p className="text-xs text-gray-500">
                      {relatedVideo.views.toLocaleString()} views • {relatedVideo.published}
                    </p>
                  </div>
                </Link>
              ))}
            </div>
          </div>
        </div>
      </div>
    </div>
  );
} 