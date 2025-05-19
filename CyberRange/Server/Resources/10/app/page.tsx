import { Header } from "@/components/Header";
import { Sidebar } from "@/components/Sidebar";
import { VideoGrid } from "@/components/VideoGrid";
import { videos } from "@/data/videos";

export default function Home() {
  // Filter videos for different sections
  const trendingVideos = videos.sort((a, b) => b.views - a.views).slice(0, 4);
  const nileVideos = videos.filter(video => video.category === 'nile');
  const alligatorVideos = videos.filter(video => video.category === 'alligators');
  
  return (
    <main className="min-h-screen bg-background">
      <Header />
      <div className="flex">
        <Sidebar />
        <div className="md:ml-60 w-full px-4 md:px-8 pt-6 pb-20">
          <section className="mb-12">
            <VideoGrid videos={trendingVideos} title="Trending" />
          </section>
          
          <section className="mb-12">
            <VideoGrid videos={nileVideos} title="Nile Crocodile Videos" />
          </section>
          
          <section className="mb-12">
            <VideoGrid videos={alligatorVideos} title="Alligator Videos" />
          </section>
        </div>
      </div>
    </main>
  );
}
