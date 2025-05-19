import { Header } from "@/components/Header";
import { Sidebar } from "@/components/Sidebar";
import Link from "next/link";
import Image from "next/image";

export default function SubscriptionsPage() {
  // Channel suggestions data
  const channelSuggestions = [
    {
      id: 1,
      name: "Wild Reptile Channel",
      avatar: "https://www.shutterstock.com/shutterstock/photos/1507332917/display_1500/stock-vector-cute-crocodile-avatar-with-yellow-background-1507332917.jpg",
      subscribers: "1.24M",
      description: "Daily videos of crocodiles and other reptiles in their natural habitat.",
    },
    {
      id: 2,
      name: "Aussie Wildlife",
      avatar: "https://www.shutterstock.com/image-vector/cute-green-crocodile-lizard-face-600nw-2453203319.jpg",
      subscribers: "895K",
      description: "Exploring Australia's amazing reptiles and wildlife conservation efforts.",
    },
    {
      id: 3,
      name: "luma crocodilu lilu 🐊",
      avatar: "https://www.shutterstock.com/image-vector/strong-crocodile-mascot-character-vector-600w-2489389405.jpg",
      subscribers: "2.1M",
      description: "Luma pwns the crocodile's life",
    },
    {
      id: 4,
      name: "Reptile Facts",
      avatar: "https://www.shutterstock.com/image-vector/icon-cute-crocodile-260nw-262283723.jpg",
      subscribers: "543K",
      description: "Scientific exploration of crocodilian evolution, behavior, and conservation.",
    },
  ];

  // Function to convert subscriber string to number for sorting
  const subscribersToNumber = (subscribers: string): number => {
    const value = parseFloat(subscribers.replace(/[^0-9.]/g, ''));
    if (subscribers.includes('K')) {
      return value * 1000;
    } else if (subscribers.includes('M')) {
      return value * 1000000;
    }
    return value;
  };

  // Sort channels by subscriber count in descending order
  const sortedChannels = [...channelSuggestions].sort((a, b) => 
    subscribersToNumber(b.subscribers) - subscribersToNumber(a.subscribers)
  );

  return (
    <div className="min-h-screen bg-background">
      <Header />
      <div className="flex">
        <Sidebar />
        <div className="md:ml-60 w-full px-4 md:px-8 pt-6 pb-20">
          <h1 className="text-3xl font-bold mb-8">Subscriptions</h1>
          
          {/* Login prompt for non-logged in users */}
          <div className="bg-gray-50 dark:bg-gray-900 rounded-xl p-8 mb-12 text-center">
            <svg 
              className="w-16 h-16 mx-auto mb-4 text-primary opacity-80" 
              fill="currentColor" 
              viewBox="0 0 24 24"
            >
              <path d="M20 4H4c-1.1 0-1.99.9-1.99 2L2 18c0 1.1.9 2 2 2h16c1.1 0 2-.9 2-2V6c0-1.1-.9-2-2-2zm0 14H4V8l8 5 8-5v10zm-8-7L4 6h16l-8 5z" />
            </svg>
            <h2 className="text-2xl font-bold mb-2">Don't miss new videos</h2>
            <p className="text-gray-600 dark:text-gray-400 mb-6">Sign in to see updates from your favorite crocodile channels</p>
            <div className="flex flex-col sm:flex-row gap-4 justify-center">
              <Link href="/signin" className="bg-primary text-white px-6 py-3 rounded-full font-medium hover:bg-opacity-90 transition-colors">
                Sign in
              </Link>
              <Link href="/signup" className="bg-transparent border border-primary text-primary px-6 py-3 rounded-full font-medium hover:bg-primary/10 transition-colors">
                Create account
              </Link>
            </div>
          </div>
          
          {/* Channel suggestions */}
          <div className="mb-10">
            <h2 className="text-xl font-bold mb-4">Channels you might like</h2>
            <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-6">
              {/* Map through sorted channel suggestions */}
              {sortedChannels.map((channel) => (
                <div key={channel.id} className="bg-white dark:bg-gray-800 rounded-xl p-5 flex flex-col items-center text-center shadow-sm">
                  <div className="relative h-24 w-24 mb-4">
                    <Image 
                      src={channel.avatar}
                      alt={`${channel.name} Avatar`}
                      fill
                      className="rounded-full object-cover"
                    />
                  </div>
                  <h3 className="font-bold text-lg mb-1">{channel.name}</h3>
                  <p className="text-gray-500 text-sm mb-3">{channel.subscribers} subscribers</p>
                  <p className="text-sm text-gray-600 dark:text-gray-400 mb-4">
                    {channel.description}
                  </p>
                  <button className="bg-gray-100 dark:bg-gray-700 hover:bg-gray-200 dark:hover:bg-gray-600 px-4 py-2 rounded-full text-sm transition-colors">
                    View Channel
                  </button>
                </div>
              ))}
            </div>
          </div>
        </div>
      </div>
    </div>
  );
} 