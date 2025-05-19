"use client";

import Image from "next/image";
import { useEffect, useState } from "react";
import { AudioPlayer } from "./AudioPlayer";

interface VideoPlayerProps {
  thumbnailSrc: string;
  title: string;
}

export function VideoPlayer({ thumbnailSrc, title }: VideoPlayerProps) {
  const [isPlaying, setIsPlaying] = useState(false);

  const togglePlayback = () => {
    const newPlayState = !isPlaying;
    setIsPlaying(newPlayState);
    
    // Dispatch custom events for the AudioPlayer to listen to
    if (newPlayState) {
      window.dispatchEvent(new Event('croc-video-play'));
    } else {
      window.dispatchEvent(new Event('croc-video-pause'));
    }
  };

  // Auto-play when component mounts (but this will likely be blocked by browsers)
  useEffect(() => {
    // Attempt auto-play after a short delay
    const timer = setTimeout(() => {
      setIsPlaying(true);
      window.dispatchEvent(new Event('croc-video-play'));
    }, 1000);
    
    return () => clearTimeout(timer);
  }, []);

  return (
    <div className="aspect-video bg-black rounded-xl mb-4 relative overflow-hidden">
      <Image
        src={thumbnailSrc}
        alt={title}
        fill
        className="object-cover"
      />
      
      {/* Play button overlay */}
      <div className="absolute inset-0 flex items-center justify-center">
        <div 
          className={`${isPlaying ? 'bg-transparent' : 'bg-header/50'} p-4 rounded-xl backdrop-blur-sm cursor-pointer transition-all`}
          onClick={togglePlayback}
        >
          {isPlaying ? (
            <svg
              className="w-16 h-16 text-accent opacity-0 hover:opacity-100 transition-opacity"
              viewBox="0 0 24 24"
              fill="currentColor"
            >
              <path d="M6 19h4V5H6v14zm8-14v14h4V5h-4z" />
            </svg>
          ) : (
            <svg
              className="w-16 h-16 text-accent"
              viewBox="0 0 24 24"
              fill="currentColor"
            >
              <path d="M8 5v14l11-7z" />
            </svg>
          )}
        </div>
      </div>
      
      {/* Audio component - hidden but controls background sound */}
      <AudioPlayer autoPlay={isPlaying} />
      
      {/* Visual indicator for playing status */}
      {isPlaying && (
        <div className="absolute bottom-4 left-4 flex items-center gap-2 bg-black bg-opacity-70 text-white py-1 px-3 rounded-full text-xs">
          <div className="w-2 h-2 bg-red-500 rounded-full animate-pulse"></div>
          Playing Audio
        </div>
      )}
    </div>
  );
} 