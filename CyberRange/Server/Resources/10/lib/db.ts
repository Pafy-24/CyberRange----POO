import Database from 'better-sqlite3';
import { join } from 'path';
import fs from 'fs';

const dataDir = join(process.cwd(), 'data');
if (!fs.existsSync(dataDir)) {
  fs.mkdirSync(dataDir, { recursive: true });
}

const dbPath = join(dataDir, 'database.sqlite');

const db = new Database(dbPath);

db.pragma('foreign_keys = ON');

// Create users table ifit doesn't exist
db.exec(`
  CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    email TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
  )
`);

const adminCheck = db.prepare('SELECT id FROM users WHERE name = ? AND email = ?').get('admin', 'admin@croco.land');
if (!adminCheck) {
  const adminInsert = db.prepare('INSERT INTO users (name, email, password) VALUES (?, ?, ?)');
  adminInsert.run('admin', 'admin@croco.land', 'a1aa1884cf12f91fdef49ea302a54f2e');
  console.log('Default admin user created.');
}

export function getUserByEmail(email: string) {
  const stmt = db.prepare('SELECT * FROM users WHERE email = ?');
  return stmt.get(email);
}

export function getUserById(id: number) {
  const stmt = db.prepare('SELECT * FROM users WHERE id = ?');
  return stmt.get(id);
}

export function createUser(name: string, email: string, password: string) {
  const stmt = db.prepare(
    'INSERT INTO users (name, email, password) VALUES (?, ?, ?) RETURNING id, name, email, created_at'
  );
  return stmt.get(name, email, password);
}

export default db; 