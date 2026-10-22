using FlagGuessingGameAPI.Models;
using H2Projekt.Infrastructure.Repositories;

namespace FlagGuessingGameAPI.Repositories
{
    public interface IPlayerRepository : IBaseRepository
    {
        Task<int> AddPlayerAsync(Player player, CancellationToken cancellationToken);
        Task<List<Player>> GetAllPlayersAsync(CancellationToken cancellationToken);
        Task<Player?> GetPlayerByIdAsync(int playerId, CancellationToken cancellationToken);
        Task<Player> GetPlayerByNameAsync(string name, CancellationToken cancellationToken);
    }
}