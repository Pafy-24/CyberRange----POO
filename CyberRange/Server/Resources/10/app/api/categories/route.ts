import { NextResponse } from 'next/server';
import { videos } from '@/data/videos';

// Define category information
const categoryInfo = {
  nile: {
    id: "nile",
    title: "Nile Crocodiles",
    description: "The Nile crocodile is a large crocodilian native to freshwater habitats in Africa. Known for their aggressive nature and powerful jaws.",
    thumbnail: "https://images.unsplash.com/photo-1610465299993-e6675c9f9efa"
  },
  saltwater: {
    id: "saltwater",
    title: "Saltwater Crocodiles",
    description: "The saltwater crocodile is the largest living reptile and crocodilian known today. Found in saltwater habitats from India to northern Australia.",
    thumbnail: "https://images.unsplash.com/photo-1534740788924-347ecb50e60e"
  },
  alligators: {
    id: "alligators",
    title: "Alligators",
    description: "Alligators are crocodilians in the genus Alligator of the family Alligatoridae. Found primarily in the southeastern United States and China.",
    thumbnail: "https://images.unsplash.com/photo-1598105729883-bc928b1afb52"
  },
  caiman: {
    id: "caiman",
    title: "Caiman",
    description: "Caimans are alligatorid crocodilians within the subfamily Caimaninae. They inhabit Central and South American wetlands.",
    thumbnail: "https://images.unsplash.com/photo-1631934859994-c0a6bbd9e815"
  },
  gharial: {
    id: "gharial",
    title: "Gharial",
    description: "The gharial is a crocodilian in the family Gavialidae. Critically endangered and native to the Indian subcontinent with a distinctive long, thin snout.",
    thumbnail: "https://images.unsplash.com/photo-1551508872-6ab074bf7a41"
  }
};

export async function GET(request: Request) {
  try {
    // Calculate video count for each category
    const categoriesWithCounts = Object.entries(categoryInfo).map(([key, category]) => {
      const videoCount = videos.filter(video => video.category === key).length;
      return {
        ...category,
        videoCount
      };
    });
    
    return NextResponse.json({
      categories: categoriesWithCounts
    }, { status: 200 });
  } catch (error) {
    return NextResponse.json(
      { error: 'Failed to fetch categories' },
      { status: 500 }
    );
  }
} 