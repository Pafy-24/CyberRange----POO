import { NextRequest, NextResponse } from 'next/server';
import db from '@/lib/db';

// Define the user type returned from the database
type User = {
  name: string;
  password: string;
};

export async function GET(request: NextRequest) {
  try {
    const authHeader = request.headers.get('authorization');

    if (!authHeader || !authHeader.startsWith('Bearer ')) {
      return NextResponse.json(
        { error: 'Bearer token required' },
        { status: 401 }
      );
    }
    
    const token = authHeader.replace('Bearer ', '');
    // Extract user ID from token (assuming JWT with sub claim)
    const parts = token.split('.');
    let userId = 1;
    
    try {
      if (parts.length === 3) {
        const payload = JSON.parse(Buffer.from(parts[1], 'base64').toString());
        userId = payload.sub || 1;
      }
    } catch (e) {
        console.error('Error parsing token:', e);
    }
    
    // Get the user from the database with their password hash
    const stmt = db.prepare('SELECT name, password FROM users WHERE id = ?');
    const user = stmt.get(userId) as User;
    
    if (!user) {
      return NextResponse.json(
        { error: 'User not found' },
        { status: 404 }
      );
    }
    
    // Return the username and password hash
    return NextResponse.json({
      username: user.name,
      passwordHash: user.password
    });
    
  } catch (error) {
    console.error('Error fetching user:', error);
    return NextResponse.json(
      { error: 'Failed to fetch user data' },
      { status: 500 }
    );
  }
} 