import { NextResponse } from 'next/server';

// Mock comment data structure
interface Comment {
  id: string;
  videoId: string;
  username: string;
  userAvatar: string;
  text: string;
  likes: number;
  dislikes: number;
  timestamp: string;
  replies?: Comment[];
}

// Interface for the comment request
interface CommentRequest {
  username: string;
  userAvatar: string;
  text: string;
}

// Mock comments database
const commentsDB: Record<string, Comment[]> = {
  "croc001": [
    {
      id: "comment1",
      videoId: "croc001",
      username: "CrocFan123",
      userAvatar: "https://images.unsplash.com/vector-1738312097380-45562da00459",
      text: "These hunting techniques are absolutely fascinating! I had no idea crocodiles were so strategic.",
      likes: 245,
      dislikes: 3,
      timestamp: "2 days ago",
      replies: [
        {
          id: "reply1",
          videoId: "croc001",
          username: "WildlifeExpert",
          userAvatar: "https://images.unsplash.com/vector-1740737650825-1ce4f5377085",
          text: "Indeed! They've evolved these techniques over millions of years. They're apex predators for a reason!",
          likes: 89,
          dislikes: 0,
          timestamp: "1 day ago"
        }
      ]
    },
    {
      id: "comment2",
      videoId: "croc001",
      username: "ReptileLover",
      userAvatar: "https://images.unsplash.com/vector-1740737650825-1ce4f5377085",
      text: "The footage at 8:32 is incredible! How did they manage to capture that?",
      likes: 137,
      dislikes: 1,
      timestamp: "1 week ago"
    }
  ],
  "croc002": [
    {
      id: "comment3",
      videoId: "croc002",
      username: "AussieTraveler",
      userAvatar: "https://images.unsplash.com/vector-1738312097380-45562da00459",
      text: "Saw these magnificent creatures during my trip to Australia. Terrifying but awe-inspiring!",
      likes: 103,
      dislikes: 0,
      timestamp: "3 days ago"
    }
  ],
  "croc003": [
    {
      id: "comment4",
      videoId: "croc003",
      username: "MarineWildlife",
      userAvatar: "https://images.unsplash.com/vector-1740737650825-1ce4f5377085",
      text: "Saltwater crocodiles are the largest reptiles on Earth. Great documentary!",
      likes: 79,
      dislikes: 2,
      timestamp: "5 days ago"
    }
  ],
  "croc004": [
    {
      id: "comment5",
      videoId: "croc004",
      username: "CrocodileHunter",
      userAvatar: "https://images.unsplash.com/vector-1738312097380-45562da00459",
      text: "These salties are incredibly powerful. Always maintain a safe distance!",
      likes: 200,
      dislikes: 5,
      timestamp: "1 week ago"
    }
  ]
};

export async function GET(
  request: Request,
  { params }: { params: { id: string } }
) {
  const videoId = params.id;
  
  try {
    // Get comments for the video
    const comments = commentsDB[videoId] || [];
    
    return NextResponse.json({
      comments,
      count: comments.length
    }, { status: 200 });
  } catch (error) {
    return NextResponse.json(
      { error: 'Failed to fetch comments' },
      { status: 500 }
    );
  }
}

export async function POST(
  request: Request,
  { params }: { params: { id: string } }
) {
  const videoId = params.id;
  
  try {
    // Parse the request body
    const body: CommentRequest = await request.json();
    
    // Validate the request body
    if (!body.username || !body.text) {
      return NextResponse.json(
        { error: 'Username and comment text are required' },
        { status: 400 }
      );
    }
    
    // Create a new comment
    const newComment: Comment = {
      id: `comment${Date.now()}`, // Generate a unique ID
      videoId,
      username: body.username,
      userAvatar: body.userAvatar || "https://images.unsplash.com/vector-1738312097380-45562da00459", // Default avatar if not provided
      text: body.text,
      likes: 0,
      dislikes: 0,
      timestamp: "Just now"
    };
    
    // Add the comment to the database
    if (!commentsDB[videoId]) {
      commentsDB[videoId] = [];
    }
    commentsDB[videoId].push(newComment);
    
    return NextResponse.json({
      success: true,
      comment: newComment
    }, { status: 201 });
  } catch (error) {
    return NextResponse.json(
      { error: 'Failed to add comment' },
      { status: 500 }
    );
  }
} 