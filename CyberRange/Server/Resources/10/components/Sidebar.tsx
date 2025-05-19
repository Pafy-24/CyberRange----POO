"use client";

import Link from "next/link";
import { usePathname } from "next/navigation";

export function Sidebar() {
  const pathname = usePathname();
  
  // Check if the current path matches a given path
  const isActive = (path: string) => {
    return pathname === path || pathname.startsWith(`${path}/`);
  };
  const categories = [
    { 
      id: "nile", 
      name: "Nile Crocodiles",
      icon: (
        <svg className="h-5 w-5" viewBox="0 0 24 24" fill="none">
          <path d="M9.06,9.06a3,3,0,1,1,4.24,4.24A3,3,0,0,1,9.06,9.06Z" fill="#8FB84A" />
          <path d="M21.71,7.57a1,1,0,0,0-1.42,0L17.07,10.8a1,1,0,0,0,0,1.41,1,1,0,0,0,1.42,0l3.22-3.22A1,1,0,0,0,21.71,7.57Z" fill="#8FB84A" />
          <path d="M19.07,2.93a1,1,0,0,0-1.42,0L14.43,6.15a1,1,0,0,0,0,1.41,1,1,0,0,0,1.42,0L19.07,4.35A1,1,0,0,0,19.07,2.93Z" fill="#8FB84A" />
          <path d="M20.22,11.78l-7.07,7.07a4,4,0,0,1-5.66,0h0L4.93,16.29a1,1,0,0,0-1.42,1.42l2.83,2.83a4,4,0,0,0,5.66,0l7.07-7.07a1,1,0,0,0-1.42-1.42Z" fill="#8FB84A" />
        </svg>
      )
    },
    { 
      id: "saltwater", 
      name: "Saltwater Crocodiles", 
      icon: (
        <svg className="h-5 w-5" viewBox="0 0 24 24" fill="none">
          <path d="M19.93,4.07h0a9.88,9.88,0,0,0-14,0l-.9.9a1,1,0,0,0,0,1.41L6.41,7.8a1,1,0,0,0,1.41,0l.9-.9a6.07,6.07,0,0,1,8.56,0h0a1,1,0,0,0,1.42,0L20.12,5.5A1,1,0,0,0,19.93,4.07Z" fill="#8FB84A" />
          <path d="M9.12,9.19l-.9.9a6.07,6.07,0,0,0,0,8.56h0a1,1,0,0,0,1.41,0l1.41-1.41a1,1,0,0,0,0-1.42h0a2.37,2.37,0,0,1,0-3.34l.9-.9a1,1,0,0,0,0-1.41L10.54,9.19A1,1,0,0,0,9.12,9.19Z" fill="#8FB84A" />
          <path d="M14.46,14.12l-.9.9a2.37,2.37,0,0,0-.68,1.68,1,1,0,0,0,1,1h5.35a1,1,0,0,0,.92-.6,1,1,0,0,0-.21-1.09l-4.07-4.07A1,1,0,0,0,14.46,14.12Z" fill="#8FB84A" />
        </svg>
      )
    },
    { 
      id: "alligators", 
      name: "Alligators",
      icon: (
        <svg className="h-5 w-5" viewBox="0 0 24 24" fill="none">
          <path d="M12,5a1,1,0,0,0,1-1V3a1,1,0,0,0-2,0V4A1,1,0,0,0,12,5Z" fill="#8FB84A" />
          <path d="M20.59,10.5A9,9,0,0,0,13,5.09V7.44a1,1,0,0,1-2,0V5.09a9,9,0,0,0-7.59,5.41,1,1,0,0,0,.5,1.32,1,1,0,0,0,1.32-.5A7,7,0,0,1,12,7a7,7,0,0,1,6.77,4.32,1,1,0,0,0,1.32.5A1,1,0,0,0,20.59,10.5Z" fill="#8FB84A" />
          <path d="M19,14c0-.3-.17-3-7-3s-7,2.7-7,3v4c0,2,1,3,3,3h8c2,0,3-1,3-3Z" fill="#8FB84A" />
        </svg>
      )
    },
    { 
      id: "caiman", 
      name: "Caiman",
      icon: (
        <svg className="h-5 w-5" viewBox="0 0 24 24" fill="none">
          <path d="M19,6.5H16.5v-1a1,1,0,0,0-1-1h-7a1,1,0,0,0-1,1v1H5a3,3,0,0,0-3,3v5a3,3,0,0,0,3,3H19a3,3,0,0,0,3-3v-5A3,3,0,0,0,19,6.5Zm-3.5,7a2,2,0,0,1-2,2H10.5a2,2,0,0,1-2-2v-1a2,2,0,0,1,2-2H13.5a2,2,0,0,1,2,2Z" fill="#8FB84A" />
        </svg>
      )
    },
    { 
      id: "gharial", 
      name: "Gharial",
      icon: (
        <svg className="h-5 w-5" viewBox="0 0 24 24" fill="none">
          <path d="M19.57,4.43a1,1,0,0,0-1.42,0L14.92,7.65a3,3,0,0,0-.87,2.12v4.46a3,3,0,0,0,.87,2.12l3.23,3.22a1,1,0,0,0,1.42,0,1,1,0,0,0,0-1.41L16.34,15a1,1,0,0,1-.29-.7V9.77a1,1,0,0,1,.29-.71l3.23-3.22A1,1,0,0,0,19.57,4.43Z" fill="#8FB84A" />
          <path d="M5.79,9.06H4.5a1,1,0,0,0,0,2H5a3,3,0,0,1,3,3v.88a3,3,0,0,0,.88,2.12l1.79,1.79a1,1,0,0,0,1.42,0,1,1,0,0,0,0-1.42L10.29,15.7a1,1,0,0,1-.29-.7V14.06a5,5,0,0,0-4.21-4.95Z" fill="#8FB84A" />
        </svg>
      )
    }
  ];

  return (
    <aside className="hidden md:block w-60 h-[calc(100vh-64px)] bg-background border-r border-gray-200 overflow-y-auto fixed top-16 left-0">
      <div className="p-4">
        <nav className="space-y-6">
          <div>
            <h3 className="font-bold mb-2 text-sm uppercase text-gray-500">Main</h3>
            <ul className="space-y-2">
              <li>
                <Link 
                  href="/" 
                  className={`flex items-center gap-3 p-2 rounded-md ${
                    isActive('/') ? 'bg-primary bg-opacity-10 text-black' : 'hover:bg-gray-100 dark:hover:bg-gray-800'
                  }`}
                >
                  <svg className="h-5 w-5 text-primary" viewBox="0 0 24 24" fill="none">
                    <path d="M10 20v-6h4v6h5v-8h3L12 3 2 12h3v8z" fill="#8FB84A" />
                  </svg>
                  <span>Home</span>
                </Link>
              </li>
              <li>
                <Link 
                  href="/trending" 
                  className={`flex items-center gap-3 p-2 rounded-md ${
                    isActive('/trending') ? 'bg-primary bg-opacity-10 text-black' : 'hover:bg-gray-100 dark:hover:bg-gray-800'
                  }`}
                >
                  <svg className="h-5 w-5 text-primary" viewBox="0 0 24 24" fill="none">
                    <path d="M17.53 11.2c-.23-.3-.5-.56-.76-.82-.65-.6-1.4-1.03-2.03-1.66-1.46-1.46-1.78-3.87-.85-5.72-.9.23-1.75.75-2.45 1.32C8.9 6.4 7.9 10.07 9.1 13.22c.04.1.08.2.08.33 0 .22-.15.42-.35.5-.22.1-.46.04-.64-.12-.06-.05-.1-.1-.15-.17-1.1-1.43-1.28-3.48-.53-5.12C5.87 10 5 12.3 5.12 14.47c.04.5.1 1 .27 1.5.14.6.4 1.2.72 1.73 1.04 1.73 2.87 2.97 4.84 3.22 2.1.27 4.35-.12 5.96-1.6 1.8-1.66 2.45-4.3 1.5-6.6l-.13-.26c-.2-.45-.47-.87-.78-1.25zm-3.1 6.3c-.28.24-.73.5-1.08.6-1.1.38-2.2-.16-2.88-.82 1.2-.28 1.9-1.16 2.1-2.05.17-.8-.14-1.46-.27-2.23-.12-.74-.1-1.37.2-2.06.15.38.35.76.58 1.06.76 1 1.95 1.44 2.2 2.8.04.14.06.28.06.43.03.82-.32 1.72-.92 2.26z" fill="#8FB84A" />
                  </svg>
                  <span>Trending</span>
                </Link>
              </li>
              <li>
                <Link 
                  href="/subscriptions" 
                  className={`flex items-center gap-3 p-2 rounded-md ${
                    isActive('/subscriptions') ? 'bg-primary bg-opacity-10 text-black' : 'hover:bg-gray-100 dark:hover:bg-gray-800'
                  }`}
                >
                  <svg className="h-5 w-5 text-primary" viewBox="0 0 24 24" fill="none">
                    <path d="M20 8H4V6h16v2zm-2-6H6v2h12V2zm4 10v8c0 1.1-.9 2-2 2H4c-1.1 0-2-.9-2-2v-8c0-1.1.9-2 2-2h16c1.1 0 2 .9 2 2zm-6 4l-6-3.27v6.53L16 16z" fill="#8FB84A" />
                  </svg>
                  <span>Subscriptions</span>
                </Link>
              </li>
            </ul>
          </div>
          
          <div>
            <h3 className="font-bold mb-2 text-sm uppercase text-gray-500">Crocodile Categories</h3>
            <ul className="space-y-2">
              {categories.map((category) => (
                <li key={category.id}>
                  <Link 
                    href={`/category/${category.id}`}
                    className={`flex items-center gap-3 p-2 rounded-md ${
                      isActive(`/category/${category.id}`) ? 'bg-primary bg-opacity-10 text-black' : 'hover:bg-gray-100 dark:hover:bg-gray-800'
                    }`}
                  >
                    {category.icon}
                    <span>{category.name}</span>
                  </Link>
                </li>
              ))}
            </ul>
          </div>
        </nav>
      </div>
    </aside>
  );
} 