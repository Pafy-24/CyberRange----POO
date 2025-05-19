"use client";

import Image from "next/image";
import Link from "next/link";

interface VideoCardProps {
  id: string;
  title: string;
  thumbnail: string;
  channelName: string;
  channelAvatar: string;
  views: number;
  published: string;
  duration: string;
}

export function VideoCard({
  id,
  title,
  thumbnail,
  channelName,
  channelAvatar,
  views,
  published,
  duration,
}: VideoCardProps) {
  return (
    <div className="group">
      <Link href={`/watch?v=${id}`} className="block relative aspect-video rounded-xl overflow-hidden mb-3">
        <Image
          src={thumbnail}
          alt={title}
          fill
          className="object-cover transition-transform group-hover:scale-105"
        />
        <div className="absolute bottom-2 right-2 bg-black bg-opacity-80 text-white text-xs px-2 py-1 rounded">
          {duration}
        </div>
      </Link>
      <div className="flex gap-3">
        <Link href={`/channel/${channelName.toLowerCase().replace(/\s+/g, '')}`} className="flex-shrink-0">
          <Image
            src={channelAvatar}
            alt={channelName}
            width={36}
            height={36}
            className="rounded-full"
          />
        </Link>
        <div>
          <h3 className="font-medium line-clamp-2 text-foreground">
            <Link href={`/watch?v=${id}`}>{title}</Link>
          </h3>
          <Link
            href={`/channel/${channelName.toLowerCase().replace(/\s+/g, '')}`}
            className="text-sm text-gray-500 hover:text-gray-700"
          >
            {channelName}
          </Link>
          <div className="text-xs text-gray-500">
            {views.toLocaleString()} views • {published}
          </div>
        </div>
      </div>
    </div>
  );
} 