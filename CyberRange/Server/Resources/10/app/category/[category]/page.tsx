import { Header } from "@/components/Header";
import { Sidebar } from "@/components/Sidebar";
import { VideoGrid } from "@/components/VideoGrid";
import { videos } from "@/data/videos";
import { notFound } from "next/navigation";

const categoryInfo = {
  nile: {
    title: "Nile Crocodiles",
    description: "The Nile crocodile is a large crocodilian native to freshwater habitats in Africa. Known for their aggressive nature and powerful jaws.",
    thumbnail: "https://images.unsplash.com/photo-1610465299993-e6675c9f9efa"
  },
  saltwater: {
    title: "Saltwater Crocodiles",
    description: "The saltwater crocodile is the largest living reptile and crocodilian known today. Found in saltwater habitats from India to northern Australia.",
    thumbnail: "https://images.unsplash.com/photo-1534740788924-347ecb50e60e"
  },
  alligators: {
    title: "Alligators",
    description: "Alligators are crocodilians in the genus Alligator of the family Alligatoridae. Found primarily in the southeastern United States and China.",
    thumbnail: "https://images.unsplash.com/photo-1598105729883-bc928b1afb52"
  },
  caiman: {
    title: "Caiman",
    description: "Caimans are alligatorid crocodilians within the subfamily Caimaninae. They inhabit Central and South American wetlands.",
    thumbnail: "https://images.unsplash.com/photo-1631934859994-c0a6bbd9e815"
  },
  gharial: {
    title: "Gharial",
    description: "The gharial is a crocodilian in the family Gavialidae. Critically endangered and native to the Indian subcontinent with a distinctive long, thin snout.",
    thumbnail: "https://images.unsplash.com/photo-1551508872-6ab074bf7a41"
  }
};

type CategoryParams = {
  params: {
    category: string;
  };
};

export default function CategoryPage({ params }: CategoryParams) {
  const { category } = params;
  
  if (!Object.keys(categoryInfo).includes(category)) {
    notFound();
  }
  
  const info = categoryInfo[category as keyof typeof categoryInfo];
  
  const categoryVideos = videos.filter(video => video.category === category);
  
  return (
    <div className="min-h-screen bg-background">
      <Header />
      <div className="flex">
        <Sidebar />
        <div className="md:ml-60 w-full px-4 md:px-8 pt-6 pb-20">
          <div 
            className="relative w-full h-40 md:h-64 rounded-xl overflow-hidden mb-8 bg-center bg-cover" 
            style={{ backgroundImage: `url(${info.thumbnail})` }}
          >
            <div className="absolute inset-0 bg-gradient-to-t from-black/70 to-transparent flex flex-col justify-end p-6">
              <h1 className="text-3xl font-bold text-white">{info.title}</h1>
              <p className="text-white/80 mt-2 max-w-2xl">{info.description}</p>
            </div>
          </div>
          
          {categoryVideos.length > 0 ? (
            <VideoGrid videos={categoryVideos} title={`${info.title} Videos`} />
          ) : (
            <div className="text-center py-12">
              <h3 className="text-xl font-medium mb-2">No videos found</h3>
              <p className="text-gray-500">We couldn't find any videos in this category right now.</p>
            </div>
          )}
        </div>
      </div>
    </div>
  );
} 