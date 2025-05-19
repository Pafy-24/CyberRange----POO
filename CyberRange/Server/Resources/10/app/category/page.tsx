import { Header } from "@/components/Header";
import { Sidebar } from "@/components/Sidebar";
import Image from "next/image";
import Link from "next/link";

// Define the types for the API responses
interface Category {
  id: string;
  title: string;
  description: string;
  thumbnail: string;
  videoCount: number;
}

async function getCategories(): Promise<Category[]> {
  // Fetch categories from the API
  const res = await fetch(`${process.env.NEXT_PUBLIC_API_URL || ''}/api/categories`, {
    cache: 'no-store' // Disable cache to get fresh data
  });
  
  if (!res.ok) {
    throw new Error('Failed to fetch categories');
  }
  
  const data = await res.json();
  return data.categories;
}

export default async function CategoriesPage() {
  const categories = await getCategories();
  
  return (
    <div className="min-h-screen bg-background">
      <Header />
      <div className="flex">
        <Sidebar />
        <div className="md:ml-60 w-full px-4 md:px-8 pt-6 pb-20">
          <h1 className="text-3xl font-bold mb-2">Crocodile Categories</h1>
          <p className="text-gray-600 dark:text-gray-400 mb-8">
            Explore different species of crocodilians from around the world
          </p>
          
          <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-6">
            {categories.map((category) => (
              <Link 
                key={category.id}
                href={`/category/${category.id}`}
                className="group bg-white dark:bg-gray-800 rounded-xl overflow-hidden shadow-sm hover:shadow-md transition-shadow"
              >
                <div className="relative h-40 overflow-hidden">
                  <Image
                    src={category.thumbnail}
                    alt={category.title}
                    fill
                    className="object-cover group-hover:scale-105 transition-transform"
                  />
                  <div className="absolute inset-0 bg-gradient-to-t from-black/70 to-transparent flex flex-col justify-end p-4">
                    <h2 className="text-xl font-bold text-white">{category.title}</h2>
                    <p className="text-white/80 text-sm">{category.videoCount} videos</p>
                  </div>
                </div>
                <div className="p-4">
                  <p className="text-sm text-gray-600 dark:text-gray-300 line-clamp-2">
                    {category.description}
                  </p>
                  <div className="mt-3 text-primary font-medium text-sm flex items-center">
                    Explore category
                    <svg className="w-4 h-4 ml-1" viewBox="0 0 20 20" fill="#8FB84A">
                      <path fillRule="evenodd" d="M12.293 5.293a1 1 0 011.414 0l4 4a1 1 0 010 1.414l-4 4a1 1 0 01-1.414-1.414L14.586 11H3a1 1 0 110-2h11.586l-2.293-2.293a1 1 0 010-1.414z" clipRule="evenodd" />
                    </svg>
                  </div>
                </div>
              </Link>
            ))}
          </div>
        </div>
      </div>
    </div>
  );
} 