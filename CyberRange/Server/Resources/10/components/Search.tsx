"use client";

import { useState } from "react";
import { useRouter } from "next/navigation";

export function Search() {
  const [searchQuery, setSearchQuery] = useState("");
  const router = useRouter();

  const handleSearch = (e: React.FormEvent) => {
    e.preventDefault();
    if (searchQuery.trim()) {
      router.push(`/search?q=${encodeURIComponent(searchQuery.trim())}`);
    }
  };

  return (
    <div className="hidden md:flex flex-1 max-w-md mx-8">
      <form onSubmit={handleSearch} className="relative w-full flex">
        <input
          type="text"
          placeholder="Search for crocodile videos..."
          className="w-full py-2 px-4 pr-10 rounded-l-full border-2 border-r-0 border-gray-300 focus:outline-none focus:border-primary text-black"
          value={searchQuery}
          onChange={(e) => setSearchQuery(e.target.value)}
        />
        <button 
          type="submit"
          className="bg-primary px-4 rounded-r-full border-2 border-gray-300 border-l-0 hover:bg-opacity-90 transition-colors flex items-center justify-center"
        >
          <svg
            className="h-5 w-5"
            fill="none"
            viewBox="0 0 24 24"
            stroke="#000000"
          >
            <path
              strokeLinecap="round"
              strokeLinejoin="round"
              strokeWidth={2}
              d="M21 21l-6-6m2-5a7 7 0 11-14 0 7 7 0 0114 0z"
            />
          </svg>
        </button>
      </form>
    </div>
  );
} 