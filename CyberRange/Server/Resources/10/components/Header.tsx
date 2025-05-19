"use client";

import Link from "next/link";
import { Search } from "@/components/Search";
import Image from "next/image";
import { useState, useEffect } from "react";
import { useRouter } from "next/navigation";

type User = {
  id: number;
  name: string;
  email: string;
};

export function Header() {
  const [mobileMenuOpen, setMobileMenuOpen] = useState(false);
  const [mobileSearchOpen, setMobileSearchOpen] = useState(false);
  const [mobileSearchQuery, setMobileSearchQuery] = useState("");
  const [user, setUser] = useState<User | null>(null);
  const [isAdmin, setIsAdmin] = useState(false);
  const router = useRouter();

  // Check if user is logged in on component mount
  useEffect(() => {
    const storedUser = localStorage.getItem('user');
    if (storedUser) {
      try {
        const userData = JSON.parse(storedUser);
        setUser(userData);
        
        // Check if the user is admin
        if (userData.name === 'admin') {
          setIsAdmin(true);
        }
      } catch (e) {
        console.error('Error parsing user data:', e);
      }
    }
  }, []);

  const handleSignOut = () => {
    // Clear auth data from localStorage
    localStorage.removeItem('authToken');
    localStorage.removeItem('user');
    
    // Clear auth cookie
    document.cookie = 'authToken=; path=/; expires=Thu, 01 Jan 1970 00:00:01 GMT; SameSite=Strict';
    
    setUser(null);
    setIsAdmin(false);
    // Redirect to home
    router.push('/');
  };

  const toggleMobileMenu = () => {
    setMobileMenuOpen(!mobileMenuOpen);
    if (mobileSearchOpen) setMobileSearchOpen(false);
  };

  const toggleMobileSearch = () => {
    setMobileSearchOpen(!mobileSearchOpen);
    if (mobileMenuOpen) setMobileMenuOpen(false);
  };

  const handleMobileSearch = (e: React.FormEvent) => {
    e.preventDefault();
    if (mobileSearchQuery.trim()) {
      router.push(`/search?q=${encodeURIComponent(mobileSearchQuery.trim())}`);
      setMobileSearchOpen(false);
      setMobileSearchQuery("");
    }
  };

  return (
    <header className="sticky top-0 z-50 w-full bg-header text-white shadow-md">
      <div className="container mx-auto px-4 py-3 flex items-center justify-between">
        <Link href="/" className="flex items-center gap-2">
          <Image
            src="/images/crocodile-svgrepo-com.svg"
            alt="Crocodile Logo"
            width={32}
            height={32}
          />
          <span className="text-xl font-bold">CrocoLand</span>
        </Link>
        
        <Search />
        
        <nav className="hidden md:flex items-center gap-6">
          <Link href="/" className="hover:text-primary transition-colors">
            Home
          </Link>
          <Link href="/trending" className="hover:text-primary transition-colors">
            Trending
          </Link>
          <Link href="/subscriptions" className="hover:text-primary transition-colors">
            Subscriptions
          </Link>
          
          {/* Admin Upload Link - Only visible to admin users */}
          {isAdmin && (
            <Link href="/admin/upload" className="text-primary hover:text-primary-dark font-medium transition-colors">
              Upload
            </Link>
          )}
          
          {user ? (
            <div className="flex items-center gap-4">
              <span className="text-primary font-medium">{user.name}</span>
              <button 
                onClick={handleSignOut}
                className="bg-transparent border border-primary text-primary hover:bg-primary/10 px-4 py-2 rounded-md font-medium transition-colors"
              >
                Sign Out
              </button>
            </div>
          ) : (
            <Link href="/signin" className="bg-primary hover:bg-opacity-80 px-4 py-2 rounded-md font-medium transition-colors">
              Sign In
            </Link>
          )}
        </nav>
        
        <div className="md:hidden flex items-center gap-3">
          <button 
            className="text-white" 
            onClick={toggleMobileSearch} 
            aria-label="Search"
          >
            <svg className="h-6 w-6" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M21 21l-6-6m2-5a7 7 0 11-14 0 7 7 0 0114 0z" stroke="#8FB84A" />
            </svg>
          </button>
          <button 
            className="text-white" 
            onClick={toggleMobileMenu} 
            aria-label="Toggle mobile menu"
          >
            <svg className="h-6 w-6" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M4 6h16M4 12h16M4 18h16" stroke="#8FB84A" />
            </svg>
          </button>
        </div>
      </div>

      {/* Mobile search overlay */}
      {mobileSearchOpen && (
        <div className="md:hidden bg-header border-t border-gray-700 animate-slideDown">
          <form onSubmit={handleMobileSearch} className="container mx-auto px-4 py-3 flex">
            <input
              type="text"
              placeholder="Search for crocodile videos..."
              className="flex-1 py-2 px-4 rounded-l-full border-2 border-r-0 border-gray-300 focus:outline-none focus:border-primary text-black"
              value={mobileSearchQuery}
              onChange={(e) => setMobileSearchQuery(e.target.value)}
              autoFocus
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
      )}

      {/* Mobile menu */}
      {mobileMenuOpen && (
        <div className="md:hidden bg-header border-t border-gray-700 animate-slideDown">
          <nav className="container mx-auto px-4 py-4 flex flex-col">
            <Link href="/" className="py-3 border-b border-gray-700 hover:text-primary transition-colors">
              Home
            </Link>
            <Link href="/trending" className="py-3 border-b border-gray-700 hover:text-primary transition-colors">
              Trending
            </Link>
            <Link href="/subscriptions" className="py-3 border-b border-gray-700 hover:text-primary transition-colors">
              Subscriptions
            </Link>
            
            {/* Admin Upload Link in mobile menu - Only visible to admin users */}
            {isAdmin && (
              <Link href="/admin/upload" className="py-3 border-b border-gray-700 text-primary hover:text-primary-dark transition-colors">
                Upload
              </Link>
            )}
            
            <div className="flex gap-3 pt-4">
              {user ? (
                <button 
                  onClick={handleSignOut}
                  className="bg-transparent border border-primary text-primary hover:bg-primary/10 px-4 py-2 rounded-md font-medium transition-colors flex-1 text-center"
                >
                  Sign Out ({user.name})
                </button>
              ) : (
                <>
                  <Link href="/signin" className="bg-primary hover:bg-opacity-80 px-4 py-2 rounded-md font-medium transition-colors flex-1 text-center">
                    Sign In
                  </Link>
                  <Link href="/signup" className="bg-transparent border border-primary text-primary hover:bg-primary/10 px-4 py-2 rounded-md font-medium transition-colors flex-1 text-center">
                    Sign Up
                  </Link>
                </>
              )}
            </div>
          </nav>
        </div>
      )}
    </header>
  );
} 