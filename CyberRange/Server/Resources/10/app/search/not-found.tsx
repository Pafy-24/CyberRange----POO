import { Header } from "@/components/Header";
import Link from "next/link";

export default function SearchNotFound() {
  return (
    <div className="min-h-screen bg-background">
      <Header />
      <div className="container mx-auto px-4 py-16 text-center max-w-2xl">
        <svg 
          className="w-20 h-20 mx-auto mb-6 text-primary" 
          fill="currentColor" 
          viewBox="0 0 24 24"
        >
          <path d="M15.5 14h-.79l-.28-.27C15.41 12.59 16 11.11 16 9.5 16 5.91 13.09 3 9.5 3S3 5.91 3 9.5 5.91 16 9.5 16c1.61 0 3.09-.59 4.23-1.57l.27.28v.79l5 4.99L20.49 19l-4.99-5zm-6 0C7.01 14 5 11.99 5 9.5S7.01 5 9.5 5 14 7.01 14 9.5 11.99 14 9.5 14z" />
        </svg>
        <h1 className="text-3xl font-bold mb-4">Search Error</h1>
        <p className="text-gray-600 dark:text-gray-400 mb-8">
          Please enter a search term to find crocodile videos.
        </p>
        <div className="space-y-4">
          <h2 className="text-xl font-semibold">Popular Categories:</h2>
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
              className="inline-flex items-center gap-2 text-primary hover:underline"
            >
              <svg 
                className="w-4 h-4" 
                fill="currentColor" 
                viewBox="0 0 24 24"
              >
                <path d="M19 12H5M12 19l-7-7 7-7" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round" />
              </svg>
              Return to Home
            </Link>
          </div>
        </div>
      </div>
    </div>
  );
} 