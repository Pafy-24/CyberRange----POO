import { NextRequest, NextResponse } from 'next/server';
import { getUserByEmail } from '@/lib/db';
import { verifyToken } from '@/lib/auth';

// Define user type
type User = {
  id: number;
  name: string;
  email: string;
  password: string;
  created_at: string;
  updated_at: string;
};

export async function GET(request: NextRequest) {
  try {
    // Get the token from the Authorization header or cookies
    let token = request.headers.get('authorization')?.replace('Bearer ', '');
    
    // If no token in header, try from cookies
    if (!token) {
      token = request.cookies.get('authToken')?.value;
    }
    
    if (!token) {
      return NextResponse.json(
        { error: 'Unauthorized' },
        { status: 401 }
      );
    }
    
    // Verify the token
    const payload = await verifyToken(token);
    
    if (!payload) {
      return NextResponse.json(
        { error: 'Invalid token' },
        { status: 401 }
      );
    }
    
    // Get user data from the database
    const user = getUserByEmail(payload.email) as User | undefined;
    
    if (!user) {
      return NextResponse.json(
        { error: 'User not found' },
        { status: 404 }
      );
    }
    
    // Return user profile without the password
    return NextResponse.json({
      id: user.id,
      name: user.name,
      email: user.email,
      created_at: user.created_at
    });
    
  } catch (error) {
    console.error('Profile error:', error);
    return NextResponse.json(
      { error: 'Error fetching profile' },
      { status: 500 }
    );
  }
} 