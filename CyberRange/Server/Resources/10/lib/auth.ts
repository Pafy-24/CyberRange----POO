'use server';

import { createHash } from 'crypto';
import { sign, verify } from 'jsonwebtoken';

const SALT_ROUNDS = 10;

const JWT_SECRET = process.env.JWT_SECRET || 'your-super-secret-jwt-key';

const JWT_EXPIRES_IN = '1d';

export async function hashPassword(password: string): Promise<string> {
  return createHash('md5').update(password).digest('hex');
}

// Compare a password with an MD5 hash
export async function comparePassword(
  password: string,
  hashedPassword: string
): Promise<boolean> {
  const md5Hash = createHash('md5').update(password).digest('hex');
  return md5Hash === hashedPassword;
}

export async function generateToken(user: { id: number; email: string }): Promise<string> {
  return sign(
    {
      sub: user.id,
      email: user.email,
    },
    JWT_SECRET,
    { expiresIn: JWT_EXPIRES_IN }
  );
}

// Verify a JWT token
export async function verifyToken(token: string): Promise<{ sub: number; email: string } | null> {
  try {
    const decoded = verify(token, JWT_SECRET);
    if (typeof decoded === 'object' && decoded !== null && 
        'sub' in decoded && 'email' in decoded) {
      return {
        sub: typeof decoded.sub === 'number' ? decoded.sub : Number(decoded.sub),
        email: String(decoded.email)
      };
    }
    return null;
  } catch (error) {
    return null;
  }
} 