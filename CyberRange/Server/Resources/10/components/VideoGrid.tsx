"use client";

import { Video } from "@/data/videos";
import { VideoCard } from "./VideoCard";

interface VideoGridProps {
  videos: Video[];
  title?: string;
}

export function VideoGrid({ videos, title }: VideoGridProps) {
  return (
    <div className="mb-8">
      {title && <h2 className="text-2xl font-bold mb-6 text-foreground">{title}</h2>}
      <div className="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-4">
        {videos.map((video) => (
          <VideoCard key={video.id} {...video} />
        ))}
      </div>
    </div>
  );
} 