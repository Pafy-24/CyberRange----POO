export interface Video {
  id: string;
  title: string;
  thumbnail: string;
  channelName: string;
  channelAvatar: string;
  views: number;
  published: string;
  duration: string;
  category: string;
}

function getRandomDuration() {
  const type = Math.floor(Math.random() * 10);
  
  if (type < 1) {
    const hours = 1 + Math.floor(Math.random() * 2);
    const minutes = Math.floor(Math.random() * 60);
    const seconds = Math.floor(Math.random() * 60);
    return `${hours}:${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
  } else if (type < 4) {
    const minutes = 10 + Math.floor(Math.random() * 50);
    const seconds = Math.floor(Math.random() * 60);
    return `${minutes}:${seconds.toString().padStart(2, '0')}`;
  } else {
    const minutes = 1 + Math.floor(Math.random() * 9);
    const seconds = Math.floor(Math.random() * 60);
    return `${minutes}:${seconds.toString().padStart(2, '0')}`;
  }
}

function getRandomPublishedDate() {
  const options = [
    "1 hour ago",
    "3 hours ago",
    "12 hours ago", 
    "1 day ago", 
    "2 days ago", 
    "3 days ago",
    "1 week ago", 
    "2 weeks ago", 
    "3 weeks ago", 
    "1 month ago", 
    "2 months ago", 
    "3 months ago",
    "6 months ago", 
    "1 year ago"
  ];
  
  return options[Math.floor(Math.random() * options.length)];
}

export const videos: Video[] = [
  {
    id: "croc001",
    title: "Amazing Nile Crocodile Hunting Techniques | Wildlife Documentary",
    thumbnail: "https://images.unsplash.com/photo-1540940046315-20e42773d0d6",
    channelName: "Wild Reptile Channel",
    channelAvatar: "https://images.unsplash.com/vector-1738312097380-45562da00459",
    views: Math.floor(Math.random() * 1000000),
    published: getRandomPublishedDate(),
    duration: getRandomDuration(),
    category: "nile"
  },
  {
    id: "croc002",
    title: "Saltwater Crocodiles: The Giant Predators of Australia",
    thumbnail: "https://images.unsplash.com/photo-1600333489678-6c6a1f0269a7",
    channelName: "Aussie Wildlife",
    channelAvatar: "https://images.unsplash.com/vector-1740737650825-1ce4f5377085",
    views: Math.floor(Math.random() * 1000000),
    published: getRandomPublishedDate(),
    duration: getRandomDuration(),
    category: "alligators"
  },
  {
    id: "croc003",
    title: "Saltwater Crocodiles: The Giant Predators of Australia",
    thumbnail: "https://images.unsplash.com/photo-1614703235158-78557a5e38fd",
    channelName: "Aussie Wildlife",
    channelAvatar: "https://images.unsplash.com/vector-1740737650825-1ce4f5377085",
    views: Math.floor(Math.random() * 1000000),
    published: getRandomPublishedDate(),
    duration: getRandomDuration(),
    category: "saltwater"
  },
  {
    id: "croc004",
    title: "Saltwater Crocodiles: The Giant Predators of Australia",
    thumbnail: "https://images.unsplash.com/photo-1605999352683-ee01111710e1",
    channelName: "Aussie Wildlife",
    channelAvatar: "https://images.unsplash.com/vector-1740737650825-1ce4f5377085",
    views: Math.floor(Math.random() * 1000000),
    published: getRandomPublishedDate(),
    duration: getRandomDuration(),
    category: "caiman"
  },
  {
    id: "croc005",
    title: "The Endangered Gharial: Conservation Efforts in India",
    thumbnail: "https://images.unsplash.com/photo-1466345959838-d8def85dbc73",
    channelName: "Conservation Channel",
    channelAvatar: "https://images.unsplash.com/vector-1738312097380-45562da00459",
    views: Math.floor(Math.random() * 500000),
    published: getRandomPublishedDate(),
    duration: getRandomDuration(),
    category: "gharial"
  },
  {
    id: "croc006",
    title: "Nile Crocodile vs Wildebeest: Amazing River Crossing",
    thumbnail: "https://images.unsplash.com/photo-1580407196238-dac33f57c410",
    channelName: "Safari Explorers",
    channelAvatar: "https://images.unsplash.com/vector-1740737650825-1ce4f5377085",
    views: Math.floor(Math.random() * 2000000),
    published: getRandomPublishedDate(),
    duration: getRandomDuration(),
    category: "nile"
  }
]; 
