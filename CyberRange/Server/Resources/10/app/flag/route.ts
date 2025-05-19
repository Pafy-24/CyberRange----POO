import { NextResponse } from 'next/server';

export async function GET() {
  // Redirect to the YouTube video with autoplay enabled and muted to ensure autoplay works
  return NextResponse.redirect('https://www.youtube.com/watch?v=6MJVSpdHhKw&autoplay=1', { status: 302 });
} 