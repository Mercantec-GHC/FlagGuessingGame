using FlagGuessingGameAPI.Models;
using H2Projekt.Infrastructure.Repositories;
using Microsoft.EntityFrameworkCore;

namespace FlagGuessingGameAPI.Repositories
{
    public class PlayerRepository : BaseRepository, IPlayerRepository
    {
        public PlayerRepository(AppDbContext appDbContext) : base(appDbContext) { }

        public async Task<int> AddPlayerAsync(Player player, CancellationToken cancellationToken)
        {
            await _appDbContext.Players.AddAsync(player, cancellationToken);

            return await _appDbContext.SaveChangesAsync(cancellationToken);
        }

        public async Task<Player?> GetPlayerByIdAsync(int playerId, CancellationToken cancellationToken)
        {
            return await _appDbContext.Players.FindAsync(playerId, cancellationToken);
        }

        public async Task<Player> GetPlayerByNameAsync(string name, CancellationToken cancellationToken)
        {
            return await _appDbContext.Players.FirstOrDefaultAsync(p => p.Name == name, cancellationToken);
        }

        public async Task<List<Player>> GetAllPlayersAsync(CancellationToken cancellationToken)
        {
            return await _appDbContext.Players.ToListAsync(cancellationToken);
        }
    }
}
