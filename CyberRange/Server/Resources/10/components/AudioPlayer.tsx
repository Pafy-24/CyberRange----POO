"use client";

import { useEffect, useRef, useState } from "react";

interface AudioPlayerProps {
  autoPlay?: boolean;
}

export function AudioPlayer({ autoPlay = true }: AudioPlayerProps) {
  const audioRef = useRef<HTMLAudioElement>(null);
  const [isPlaying, setIsPlaying] = useState(false);
  const [hasError, setHasError] = useState(false);

  useEffect(() => {
    if (autoPlay) {
      playAudio();
    }

    // Listen for video play/pause events from the parent
    const handleVideoPlay = () => playAudio();
    const handleVideoPause = () => pauseAudio();

    // Add global event listeners for custom events from the video player
    window.addEventListener('croc-video-play', handleVideoPlay);
    window.addEventListener('croc-video-pause', handleVideoPause);

    return () => {
      // Cleanup event listeners
      window.removeEventListener('croc-video-play', handleVideoPlay);
      window.removeEventListener('croc-video-pause', handleVideoPause);
    };
  }, [autoPlay]);

  // Setup error handling for the audio element
  useEffect(() => {
    if (!audioRef.current) return;

    const handleError = (e: ErrorEvent) => {
      console.error('Audio playback error:', e);
      setHasError(true);
      setIsPlaying(false);
    };

    const handleCanPlay = () => {
      setHasError(false);
      if (autoPlay) playAudio();
    };

    audioRef.current.addEventListener('error', handleError as any);
    audioRef.current.addEventListener('canplay', handleCanPlay);

    return () => {
      if (audioRef.current) {
        audioRef.current.removeEventListener('error', handleError as any);
        audioRef.current.removeEventListener('canplay', handleCanPlay);
      }
    };
  }, [audioRef, autoPlay]);

  const playAudio = () => {
    if (audioRef.current && !hasError) {
      audioRef.current.play()
        .then(() => {
          setIsPlaying(true);
        })
        .catch(error => {
          console.error("Audio playback failed:", error);
          // Most browsers require user interaction before playing audio
          console.log("Audio playback requires user interaction");
        });
    }
  };

  const pauseAudio = () => {
    if (audioRef.current) {
      audioRef.current.pause();
      setIsPlaying(false);
    }
  };

  return (
    <>
      <audio 
        ref={audioRef}
        src="/crocodilu.mp3" 
        loop 
        className="hidden"
        preload="auto"
      />
      {/* Optional control for debugging */}
      {(process.env.NODE_ENV === 'development' || hasError) && (
        <div className="absolute bottom-4 right-4 z-10">
          <button 
            onClick={isPlaying ? pauseAudio : playAudio}
            className={`bg-black bg-opacity-70 text-white p-2 rounded-full ${hasError ? 'bg-red-600' : ''}`}
            disabled={hasError}
          >
            {hasError 
              ? 'Audio Error' 
              : isPlaying ? 'Pause Audio' : 'Play Audio'}
          </button>
        </div>
      )}
    </>
  );
} 