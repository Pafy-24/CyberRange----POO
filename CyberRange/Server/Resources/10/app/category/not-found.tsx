import { Header } from "@/components/Header";
import Link from "next/link";

export default function CategoryNotFound() {
  return (
    <div className="min-h-screen bg-background">
      <Header />
      <div className="container mx-auto px-4 py-16 text-center max-w-2xl">
        <h1 className="text-3xl font-bold mb-4">Category Not Found</h1>
        <p className="text-gray-600 dark:text-gray-400 mb-8">
          The crocodile category you're looking for doesn't exist or may have moved to a different habitat.
        </p>
        <div className="space-y-4">
          <h2 className="text-xl font-semibold">Available Categories:</h2>
          <div className="grid grid-cols-2 md:grid-cols-3 gap-4">
            <Link 
              href="/category/nile" 
              className="bg-primary text-white px-4 py-3 rounded-lg hover:bg-opacity-90 transition-colors"
            >
              Nile Crocodiles
            </Link>
            <Link 
              href="/category/saltwater" 
              className="bg-primary text-white px-4 py-3 rounded-lg hover:bg-opacity-90 transition-colors"
            >
              Saltwater Crocodiles
            </Link>
            <Link 
              href="/category/alligators" 
              className="bg-primary text-white px-4 py-3 rounded-lg hover:bg-opacity-90 transition-colors"
            >
              Alligators
            </Link>
            <Link 
              href="/category/caiman" 
              className="bg-primary text-white px-4 py-3 rounded-lg hover:bg-opacity-90 transition-colors"
            >
              Caiman
            </Link>
            <Link 
              href="/category/gharial" 
              className="bg-primary text-white px-4 py-3 rounded-lg hover:bg-opacity-90 transition-colors"
            >
              Gharial
            </Link>
          </div>
          <div className="mt-8">
            <Link 
              href="/" 
              className="inline-block border border-primary text-primary px-6 py-3 rounded-lg hover:bg-primary hover:text-white transition-colors"
            >
              Return to Homepage
            </Link>
          </div>
        </div>
      </div>
    </div>
  );
} 