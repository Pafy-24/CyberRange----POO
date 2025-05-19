import { NextRequest, NextResponse } from 'next/server';
import { verifyToken } from './lib/auth';

const publicRoutes = [
  '/',
  '/signin',
  '/signup',
  '/api/auth/signin',
  '/api/auth/signup',
];

const publicRoutePrefixes = [
  '/images/',
  '/_next/',
  '/favicon.ico',
];

export function middleware(request: NextRequest) {
  const { pathname } = request.nextUrl;
  
  if (publicRoutes.includes(pathname) || 
      publicRoutePrefixes.some(prefix => pathname.startsWith(prefix))) {
    return NextResponse.next();
  }
  
  const token = request.headers.get('authorization')?.replace('Bearer ', '');
  
  if (!token || !verifyToken(token)) {
    if (pathname.startsWith('/api/')) {
      return NextResponse.json(
        { error: 'Unauthorized' },
        { status: 401 }
      );
    }
    
    const signinUrl = new URL('/signin', request.url);
    signinUrl.searchParams.set('redirect', pathname);
    return NextResponse.redirect(signinUrl);
  }
  
  return NextResponse.next();
}

export const config = {
  matcher: "/api/:path*",
};